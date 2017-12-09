#include "Knife.h"
#include "../Raven_Bot.h"
#include "misc/Cgdi.h"
#include "../Raven_Game.h"
#include "../Raven_Map.h"
#include "../lua/Raven_Scriptor.h"
#include "fuzzy/FuzzyOperators.h"
#include "../debug/DebugConsole.h"

#include "../Raven_Messages.h"
#include "Messaging/MessageDispatcher.h"


#define KNIFE_DEBUG

Knife::Knife(Raven_Bot *owner):
	Raven_Weapon(type_knife,
	script->GetInt("Knife_DefaultRounds"),
	script->GetInt("Knife_MaxRoundsCarried"),
	script->GetDouble("Knife_FiringFreq"),
	script->GetDouble("Knife_IdealRange"),
	script->GetDouble("Slug_MaxSpeed"),
	owner)
{

	_maxDist = script->GetDouble("Knife_MaxDist");
	_dmg = script->GetInt("Knife_Damage");

	const int NumWeaponVerts = 3;
	const Vector2D weapon[NumWeaponVerts] = { Vector2D(0, -1),
		Vector2D(10, -1),
		Vector2D(10, 0)
	};


	for (int vtx = 0; vtx<NumWeaponVerts; ++vtx)
	{
		m_vecWeaponVB.push_back(weapon[vtx]);
	}

	InitializeFuzzyModule();
}

void Knife::Render() {
	m_vecWeaponVBTrans = WorldTransform(m_vecWeaponVB,
		m_pOwner->Pos(),
		m_pOwner->Facing(),
		m_pOwner->Facing().Perp(),
		m_pOwner->Scale());

	gdi->RedPen();

	gdi->ClosedShape(m_vecWeaponVBTrans);
}

double Knife::GetDesirability(double DistToTarget) {
	m_FuzzyModule.Fuzzify("DistToTarget", DistToTarget);

	m_dLastDesirabilityScore = m_FuzzyModule.DeFuzzify("Desirability", FuzzyModule::max_av);

	return m_dLastDesirabilityScore;
}

inline void Knife::ShootAt(Vector2D pos) {
	
#ifdef KNIFE_DEBUG
	debug_con << "Is Slash ?" << "";
#endif
	if (isReadyForNextShot())
	{
		//Shiv them all!
		std::list<Raven_Bot*> hits;
#ifdef KNIFE_DEBUG
		debug_con << "Throw Slash" << "";
#endif
		Vector2D slash;

		slash = m_pOwner->Facing();
		slash.Normalize();
		slash *= _maxDist;
		slash += m_pOwner->Pos();

		std::list<Raven_Bot*>::const_iterator curBot;
		for (curBot = m_pOwner->GetWorld()->GetAllBots().begin();
			curBot != m_pOwner->GetWorld()->GetAllBots().end();
			++curBot)
		{
			//make sure we don't check against the shooter of the projectile
			if (((*curBot)->ID() != m_pOwner->ID()))
			{
				//if the distance to FromTo is less than the entities bounding radius then
				//there is an intersection so add it to hits
				if (DistToLineSegment(m_pOwner->Pos(), slash, (*curBot)->Pos()) < (*curBot)->BRadius())
				{
					hits.push_back(*curBot);
				}
			}

		}

		//Do Damage
			//TODO DAMAGE
		std::list<Raven_Bot*>::iterator it;
		for (it = hits.begin(); it != hits.end(); ++it)
		{
			//send a message to the bot to let it know it's been hit, and who the
			//shot came from
			Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY,
				m_pOwner->ID(),
				(*it)->ID(),
				Msg_TakeThatMF,
				(void*)&_dmg);
#ifdef KNIFE_DEBUG
			debug_con << "SLASH HIT !" << "";
#endif
		}
		// Update Time

		UpdateTimeWeaponIsNextAvailable();

		//add a trigger to the game so that the other bots can hear this shot
		//(provided they are within range)
		m_pOwner->GetWorld()->GetMap()->AddSoundTrigger(m_pOwner, script->GetDouble("Knife_SoundRange"));
	}
}

void Knife::InitializeFuzzyModule() {
	FuzzyVariable& DistToTarget = m_FuzzyModule.CreateFLV("DistToTarget");

	FzSet& Target_SuperClose = DistToTarget.AddLeftShoulderSet("Target_Close", 0, 1, 20);
	FzSet& Target_Close = DistToTarget.AddLeftShoulderSet("Target_Close", 20, 25, 150);
	FzSet& Target_Medium = DistToTarget.AddTriangularSet("Target_Medium", 25, 150, 300);
	FzSet& Target_Far = DistToTarget.AddRightShoulderSet("Target_Far", 150, 300, 1000);

	FuzzyVariable& Desirability = m_FuzzyModule.CreateFLV("Desirability");
	FzSet& VeryDesirable = Desirability.AddRightShoulderSet("VeryDesirable", 50, 75, 100);
	FzSet& Desirable = Desirability.AddTriangularSet("Desirable", 25, 50, 75);
	FzSet& Undesirable = Desirability.AddLeftShoulderSet("Undesirable", 0, 25, 50);

	m_FuzzyModule.AddRule(Target_SuperClose, VeryDesirable);
	m_FuzzyModule.AddRule(Target_Close, FzVery(Undesirable));
	m_FuzzyModule.AddRule(Target_Medium, FzVery(Undesirable));
	m_FuzzyModule.AddRule(Target_Far, FzVery(Undesirable));
}

