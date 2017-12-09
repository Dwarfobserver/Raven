#include "Weapon_SlowingGun.h"
#include "../Raven_Bot.h"
#include "misc/Cgdi.h"
#include "../Raven_Game.h"
#include "../Raven_Map.h"
#include "../lua/Raven_Scriptor.h"
#include "misc/utils.h"
#include "fuzzy/FuzzyOperators.h"


//--------------------------- ctor --------------------------------------------
//-----------------------------------------------------------------------------
SlowingGun::SlowingGun(Raven_Bot*   owner) :

	Raven_Weapon(type_slowering_gun,
		script->GetInt("SloweringGun_DefaultRounds"),
		script->GetInt("SloweringGun_MaxRoundsCarried"),
		script->GetDouble("SloweringGun_FiringFreq"),
		script->GetDouble("SloweringGun_IdealRange"),
		script->GetDouble("SlowRay_MaxSpeed"),
		owner)
{

	//setup the vertex buffer
	const int NumWeaponVerts = 4;
	const Vector2D weapon[NumWeaponVerts] = { Vector2D(0, -1),
		Vector2D(10, -1),
		Vector2D(10, 1),
		Vector2D(0, 1)
	};


	for (int vtx = 0; vtx<NumWeaponVerts; ++vtx)
	{
		m_vecWeaponVB.push_back(weapon[vtx]);
	}

	//setup the fuzzy module
	InitializeFuzzyModule();

}

//------------------------------ ShootAt --------------------------------------

inline void SlowingGun::ShootAt(Vector2D pos)
{
	if (NumRoundsRemaining() > 0 && isReadyForNextShot())
	{
		// We fire
		m_pOwner->GetWorld()->AddSlowRay(m_pOwner, pos);

		// Cooldown update
		UpdateTimeWeaponIsNextAvailable();

		m_iNumRoundsLeft--;

		//add a trigger to the game so that the other bots can hear this shot
		//(provided they are within range)
		m_pOwner->GetWorld()->GetMap()->AddSoundTrigger(m_pOwner, script->GetDouble("SloweringGun_SoundRange"));

	}

}

//---------------------------- Desirability -----------------------------------
//
//-----------------------------------------------------------------------------
inline double SlowingGun::GetDesirability(double DistToTarget)
{
	if (m_iNumRoundsLeft == 0)
	{
		m_dLastDesirabilityScore = 0;
	}
	else
	{
		//fuzzify distance and amount of ammo
		m_FuzzyModule.Fuzzify("DistanceToTarget", DistToTarget);
		m_FuzzyModule.Fuzzify("AmmoStatus", (double)m_iNumRoundsLeft);

		m_dLastDesirabilityScore = m_FuzzyModule.DeFuzzify("Desirability", FuzzyModule::max_av);
	}

	return m_dLastDesirabilityScore;
}

//--------------------------- InitializeFuzzyModule ---------------------------
//
//  set up some fuzzy variables and rules
//-----------------------------------------------------------------------------
void SlowingGun::InitializeFuzzyModule()
{
	FuzzyVariable& DistanceToTarget = m_FuzzyModule.CreateFLV("DistanceToTarget");

	FzSet& Target_Close = DistanceToTarget.AddLeftShoulderSet("Target_Close", 0, 25, 150);
	FzSet& Target_Medium = DistanceToTarget.AddTriangularSet("Target_Medium", 25, 150, 300);
	FzSet& Target_Far = DistanceToTarget.AddRightShoulderSet("Target_Far", 150, 300, 1000);

	FuzzyVariable& Desirability = m_FuzzyModule.CreateFLV("Desirability");

	FzSet& VeryDesirable = Desirability.AddRightShoulderSet("VeryDesirable", 50, 75, 100);
	FzSet& Desirable = Desirability.AddTriangularSet("Desirable", 25, 50, 75);
	FzSet& Undesirable = Desirability.AddLeftShoulderSet("Undesirable", 0, 25, 50);

	FuzzyVariable& AmmoStatus = m_FuzzyModule.CreateFLV("AmmoStatus");
	FzSet& Ammo_Loads = AmmoStatus.AddRightShoulderSet("Ammo_Loads", 30, 60, 100);
	FzSet& Ammo_Okay = AmmoStatus.AddTriangularSet("Ammo_Okay", 0, 30, 60);
	FzSet& Ammo_Low = AmmoStatus.AddTriangularSet("Ammo_Low", 0, 0, 30);


	m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Loads), Undesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Okay), Desirable);
	m_FuzzyModule.AddRule(FzAND(Target_Close, Ammo_Low), VeryDesirable);

	m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Loads), Desirable);
	m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Okay), Desirable);
	m_FuzzyModule.AddRule(FzAND(Target_Medium, Ammo_Low), VeryDesirable);

	m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Loads), Undesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Okay), Undesirable);
	m_FuzzyModule.AddRule(FzAND(Target_Far, Ammo_Low), Desirable);
}

//-------------------------------- Render -------------------------------------
//-----------------------------------------------------------------------------
void SlowingGun::Render()
{
	m_vecWeaponVBTrans = WorldTransform(m_vecWeaponVB,
		m_pOwner->Pos(),
		m_pOwner->Facing(),
		m_pOwner->Facing().Perp(),
		m_pOwner->Scale());

	gdi->BrownPen();

	gdi->PolyLine(m_vecWeaponVBTrans);

}