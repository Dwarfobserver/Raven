#include "AttackTargetGoal_Evaluator.h"
#include "Goal_Think.h"
#include "Raven_Goal_Types.h"
#include "../Raven_WeaponSystem.h"
#include "../Raven_ObjectEnumerations.h"
#include "misc/cgdi.h"
#include "misc/Stream_Utility_Functions.h"
#include "Raven_Feature.h"
#include "../Raven_Game.h"
#include "../Raven_Map.h"

#include "debug/DebugConsole.h"
#include "2d/WallIntersectionTests.h"

//------------------ CalculateDesirability ------------------------------------
//
//  returns a value between 0 and 1 that indicates the Rating of a bot (the
//  higher the score, the stronger the bot).
//-----------------------------------------------------------------------------
double AttackTargetGoal_Evaluator::CalculateDesirability(Raven_Bot* pBot)
{
  double Desirability = 0.0;

  //only do the calculation if there is a target present
  if (pBot->GetTargetSys()->isTargetPresent()) 
  {
	  const auto healthRatio = Raven_Feature::Health(pBot);
	  const auto weaponsRatio = Raven_Feature::TotalWeaponStrength(pBot);


	  if (pBot->isClumsy())
	  {
		  auto pWorld = pBot->GetWorld();
		  auto pTarget = pBot->GetTargetBot();

		  double distance = Vec2DDistanceSq(pBot->Pos(), pTarget->Pos());

		  bool sight= doWallsObstructLineSegment(
			  pBot->Pos(),
			  pTarget->Pos(),
			  pWorld->GetMap()->GetWalls());

		  al::Record r;
		  r[al::Attributes::TargetDistance] = distance;
		  r[al::Attributes::LineOfSight] = !sight ? 1.0 : 0.0;
		  r[al::Attributes::OwnerLife] = healthRatio;
		  r[al::Attributes::AmmoCount] = weaponsRatio;

		  Desirability = pWorld->neuralNetwork.evaluate(r);
	  }
	  else {
		  const double Tweaker = 1.0;

		  Desirability = Tweaker * healthRatio * weaponsRatio;

	  }

	  //bias the value according to the personality of the bot
	  Desirability *= m_dCharacterBias;
     
  }
    
  return Desirability;
}

//----------------------------- SetGoal ---------------------------------------
//-----------------------------------------------------------------------------
void AttackTargetGoal_Evaluator::SetGoal(Raven_Bot* pBot)
{
  pBot->GetBrain()->AddGoal_AttackTarget(); 
}

//-------------------------- RenderInfo ---------------------------------------
//-----------------------------------------------------------------------------
void AttackTargetGoal_Evaluator::RenderInfo(Vector2D Position, Raven_Bot* pBot)
{
  gdi->TextAtPos(Position, "AT: " + ttos(CalculateDesirability(pBot), 2));
  return;
    
  std::string s = ttos(Raven_Feature::Health(pBot)) + ", " + ttos(Raven_Feature::TotalWeaponStrength(pBot));
  gdi->TextAtPos(Position+Vector2D(0,12), s);
}