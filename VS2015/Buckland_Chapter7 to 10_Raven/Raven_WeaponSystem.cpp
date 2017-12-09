#include "Raven_WeaponSystem.h"
#include "armory/Weapon_RocketLauncher.h"
#include "armory/Weapon_RailGun.h"
#include "armory/Weapon_ShotGun.h"
#include "armory/Weapon_Blaster.h"
#include "armory/Weapon_SlowingGun.h"
#include "Raven_Bot.h"
#include "misc/utils.h"
#include "lua/Raven_Scriptor.h"
#include "Raven_Game.h"
#include "Raven_UserOptions.h"
#include "2D/transformations.h"
#include "debug/DebugConsole.h"

//------------------------- ctor ----------------------------------------------
//-----------------------------------------------------------------------------
Raven_WeaponSystem::Raven_WeaponSystem(Raven_Bot* owner,
                                       double ReactionTime,
                                       double AimAccuracy,
                                       double AimPersistance):m_pOwner(owner),
                                                          m_dReactionTime(ReactionTime),
                                                          m_dAimAccuracy(AimAccuracy),
                                                          m_dAimPersistance(AimPersistance)
{
  Initialize();

  //setup the fuzzy module
  InitializeFuzzyModule();
}

//------------------------- dtor ----------------------------------------------
//-----------------------------------------------------------------------------
Raven_WeaponSystem::~Raven_WeaponSystem()
{
  for (unsigned int w=0; w<m_WeaponMap.size(); ++w)
  {
    delete m_WeaponMap[w];
  }
}

//------------------------------ Initialize -----------------------------------
//
//  initializes the weapons
//-----------------------------------------------------------------------------
void Raven_WeaponSystem::Initialize()
{
  //delete any existing weapons
  WeaponMap::iterator curW;
  for (curW = m_WeaponMap.begin(); curW != m_WeaponMap.end(); ++curW)
  {
    delete curW->second;
  }

  m_WeaponMap.clear();

  //set up the container
  m_pCurrentWeapon = new Blaster(m_pOwner);

  m_WeaponMap[type_blaster]         = m_pCurrentWeapon;
  m_WeaponMap[type_shotgun]         = 0;
  m_WeaponMap[type_rail_gun]        = 0;
  m_WeaponMap[type_rocket_launcher] = 0;
  m_WeaponMap[type_slowering_gun] = 0;
}

//-------------------------------- SelectWeapon -------------------------------
//
//-----------------------------------------------------------------------------
void Raven_WeaponSystem::SelectWeapon()
{ 
  //if a target is present use fuzzy logic to determine the most desirable 
  //weapon.
  if (m_pOwner->GetTargetSys()->isTargetPresent())
  {
    //calculate the distance to the target
    double DistToTarget = Vec2DDistance(m_pOwner->Pos(), m_pOwner->GetTargetSys()->GetTarget()->Pos());

    //for each weapon in the inventory calculate its desirability given the 
    //current situation. The most desirable weapon is selected
    double BestSoFar = MinDouble;

    WeaponMap::const_iterator curWeap;
    for (curWeap=m_WeaponMap.begin(); curWeap != m_WeaponMap.end(); ++curWeap)
    {
      //grab the desirability of this weapon (desirability is based upon
      //distance to target and ammo remaining)
      if (curWeap->second)
      {
        double score = curWeap->second->GetDesirability(DistToTarget);

        //if it is the most desirable so far select it
        if (score > BestSoFar)
        {
          BestSoFar = score;

          //place the weapon in the bot's hand.
          m_pCurrentWeapon = curWeap->second;
        }
      }
    }
  }

  else
  {
    m_pCurrentWeapon = m_WeaponMap[type_blaster];
  }
}

//--------------------  AddWeapon ------------------------------------------
//
//  this is called by a weapon affector and will add a weapon of the specified
//  type to the bot's inventory.
//
//  if the bot already has a weapon of this type then only the ammo is added
//-----------------------------------------------------------------------------
void  Raven_WeaponSystem::AddWeapon(unsigned int weapon_type)
{
  //create an instance of this weapon
  Raven_Weapon* w = 0;

  switch(weapon_type)
  {
  case type_rail_gun:

    w = new RailGun(m_pOwner); break;

  case type_shotgun:

    w = new ShotGun(m_pOwner); break;

  case type_rocket_launcher:

    w = new RocketLauncher(m_pOwner); break;

  case type_slowering_gun:

	  w = new SlowingGun(m_pOwner); break;
  }//end switch
  

  //if the bot already holds a weapon of this type, just add its ammo
  Raven_Weapon* present = GetWeaponFromInventory(weapon_type);

  if (present)
  {
    present->IncrementRounds(w->NumRoundsRemaining());

    delete w;
  }
  
  //if not already holding, add to inventory
  else
  {
    m_WeaponMap[weapon_type] = w;
  }
}


//------------------------- GetWeaponFromInventory -------------------------------
//
//  returns a pointer to any matching weapon.
//
//  returns a null pointer if the weapon is not present
//-----------------------------------------------------------------------------
Raven_Weapon* Raven_WeaponSystem::GetWeaponFromInventory(int weapon_type)
{
  return m_WeaponMap[weapon_type];
}

//----------------------- ChangeWeapon ----------------------------------------
void Raven_WeaponSystem::ChangeWeapon(unsigned int type)
{
  Raven_Weapon* w = GetWeaponFromInventory(type);

  if (w) m_pCurrentWeapon = w;
}

//--------------------------- TakeAimAndShoot ---------------------------------
//
//  this method aims the bots current weapon at the target (if there is a
//  target) and, if aimed correctly, fires a round
//-----------------------------------------------------------------------------
void Raven_WeaponSystem::TakeAimAndShoot()
{
  //aim the weapon only if the current target is shootable or if it has only
  //very recently gone out of view (this latter condition is to ensure the 
  //weapon is aimed at the target even if it temporarily dodges behind a wall
  //or other cover)
  if (m_pOwner->GetTargetSys()->isTargetShootable() ||
      (m_pOwner->GetTargetSys()->GetTimeTargetHasBeenOutOfView() < 
       m_dAimPersistance) )
  {
    //the position the weapon will be aimed at
    Vector2D AimingPos = m_pOwner->GetTargetBot()->Pos();


    //if the current weapon is not an instant hit type gun the target position
    //must be adjusted to take into account the predicted movement of the 
    //target
    if (GetCurrentWeapon()->GetType() == type_rocket_launcher ||
        GetCurrentWeapon()->GetType() == type_blaster || GetCurrentWeapon()->GetType() == type_slowering_gun)
    {
		AimingPos = PredictFuturePositionOfTarget();

      //if the weapon is aimed correctly, there is line of sight between the
      //bot and the aiming position and it has been in view for a period longer
      //than the bot's reaction time, shoot the weapon
      if ( m_pOwner->RotateFacingTowardPosition(AimingPos) &&
           (m_pOwner->GetTargetSys()->GetTimeTargetHasBeenVisible() >
            m_dReactionTime) &&
           m_pOwner->hasLOSto(AimingPos) )
      {
        AddNoiseToAim(AimingPos);

        GetCurrentWeapon()->ShootAt(AimingPos);
      }
    }

    //no need to predict movement, aim directly at target
    else
    {
      //if the weapon is aimed correctly and it has been in view for a period
      //longer than the bot's reaction time, shoot the weapon
      if ( m_pOwner->RotateFacingTowardPosition(AimingPos) &&
           (m_pOwner->GetTargetSys()->GetTimeTargetHasBeenVisible() >
            m_dReactionTime) )
      {
        AddNoiseToAim(AimingPos);
        
        GetCurrentWeapon()->ShootAt(AimingPos);
      }
    }

  }
  
  //no target to shoot at so rotate facing to be parallel with the bot's
  //heading direction
  else
  {
    m_pOwner->RotateFacingTowardPosition(m_pOwner->Pos()+ m_pOwner->Heading());
  }
}

//---------------------------- AddNoiseToAim ----------------------------------
//
//  adds a random deviation to the firing angle not greater than m_dAimAccuracy 
//  rads
//-----------------------------------------------------------------------------
void Raven_WeaponSystem::AddNoiseToAim(Vector2D& AimingPos)
{
  Vector2D toPos = AimingPos - m_pOwner->Pos();

  //fuzzify distance, Velocity and time Visible
  fuzzyModule.Fuzzify("DistToTarget", toPos.Length());
  fuzzyModule.Fuzzify("TargetVelocity", m_pOwner->GetTargetBot()->Speed());
  fuzzyModule.Fuzzify("VisibleTime", m_pOwner->GetTargetSys()->GetTimeTargetHasBeenVisible());

  double noiseToAdd = fuzzyModule.DeFuzzify("NoiseToAdd", FuzzyModule::max_av);

  Vec2DRotateAroundOrigin(toPos, RandInRange(-m_dAimAccuracy - noiseToAdd, m_dAimAccuracy + noiseToAdd));

  AimingPos = toPos + m_pOwner->Pos();
}

void Raven_WeaponSystem::InitializeFuzzyModule()
{
	// initialisation du FuzzyModule
	FuzzyVariable& NoiseToAdd = fuzzyModule.CreateFLV("NoiseToAdd");

	FzSet& VeryAccurate = NoiseToAdd.AddLeftShoulderSet("VeryAccurate", 0.0, 0.0, 0.1);
	FzSet& Accurate = NoiseToAdd.AddTriangularSet("Accurate", 0.0, 0.1, 0.3);
	FzSet& Unaccurate = NoiseToAdd.AddRightShoulderSet("Unaccurate", 0.1, 0.3, 0.5);

	FuzzyVariable& DistToTarget = fuzzyModule.CreateFLV("DistToTarget");

	FzSet& Target_Close = DistToTarget.AddTriangularSet("Target_Close", 0, 25, 150);
	FzSet& Target_Medium = DistToTarget.AddTriangularSet("Target_Medium", 25, 150, 300);
	FzSet& Target_Far = DistToTarget.AddTriangularSet("Target_Far", 150, 300, 1000);

	FuzzyVariable& TargetVelocity = fuzzyModule.CreateFLV("TargetVelocity");

	FzSet& VeryFast = TargetVelocity.AddRightShoulderSet("VeryFast", 0.95, 1.0, 10.0);
	FzSet& Fast = TargetVelocity.AddTriangularSet("Fast", 0.75, 0.8, 1.0);
	FzSet& Slow = TargetVelocity.AddLeftShoulderSet("Slow", 0.0, 0.75, 0.8);

	FuzzyVariable& VisibleTime = fuzzyModule.CreateFLV("VisibleTime");

	FzSet& Visible_Long = VisibleTime.AddRightShoulderSet("Visible_Long", 2.0, 3.0, 100.0);
	FzSet& Visible_Medium = VisibleTime.AddTriangularSet("Visible_Medium", 1.0, 2.0, 3.0);
	FzSet& Visible_Brief = VisibleTime.AddLeftShoulderSet("Visible_Brief", 0.0, 1.0, 2.0);

	// ajout des règles

	fuzzyModule.AddRule(FzAND(Target_Close, VeryFast, Visible_Long), Accurate);
	fuzzyModule.AddRule(FzAND(Target_Close, VeryFast, Visible_Medium), Accurate);
	fuzzyModule.AddRule(FzAND(Target_Close, VeryFast, Visible_Brief), Accurate);

	fuzzyModule.AddRule(FzAND(Target_Close, Fast, Visible_Long), VeryAccurate);
	fuzzyModule.AddRule(FzAND(Target_Close, Fast, Visible_Medium), Accurate);
	fuzzyModule.AddRule(FzAND(Target_Close, Fast, Visible_Brief), Accurate);

	fuzzyModule.AddRule(FzAND(Target_Close, Slow, Visible_Long), VeryAccurate);
	fuzzyModule.AddRule(FzAND(Target_Close, Slow, Visible_Medium), VeryAccurate);
	fuzzyModule.AddRule(FzAND(Target_Close, Slow, Visible_Brief), Accurate);

	fuzzyModule.AddRule(FzAND(Target_Medium, VeryFast, Visible_Long), Accurate);
	fuzzyModule.AddRule(FzAND(Target_Medium, VeryFast, Visible_Medium), Accurate);
	fuzzyModule.AddRule(FzAND(Target_Medium, VeryFast, Visible_Brief), Unaccurate);

	fuzzyModule.AddRule(FzAND(Target_Medium, Fast, Visible_Long), Accurate);
	fuzzyModule.AddRule(FzAND(Target_Medium, Fast, Visible_Medium), Accurate);
	fuzzyModule.AddRule(FzAND(Target_Medium, Fast, Visible_Brief), Accurate);

	fuzzyModule.AddRule(FzAND(Target_Medium, Slow, Visible_Long), VeryAccurate);
	fuzzyModule.AddRule(FzAND(Target_Medium, Slow, Visible_Medium), Accurate);
	fuzzyModule.AddRule(FzAND(Target_Medium, Slow, Visible_Brief), Accurate);

	fuzzyModule.AddRule(FzAND(Target_Far, VeryFast, Visible_Long), Accurate);
	fuzzyModule.AddRule(FzAND(Target_Far, VeryFast, Visible_Medium), Unaccurate);
	fuzzyModule.AddRule(FzAND(Target_Far, VeryFast, Visible_Brief), Unaccurate);

	fuzzyModule.AddRule(FzAND(Target_Far, Fast, Visible_Long), Accurate);
	fuzzyModule.AddRule(FzAND(Target_Far, Fast, Visible_Medium), Accurate);
	fuzzyModule.AddRule(FzAND(Target_Far, Fast, Visible_Brief), Unaccurate);

	fuzzyModule.AddRule(FzAND(Target_Far, Slow, Visible_Long), Accurate);
	fuzzyModule.AddRule(FzAND(Target_Far, Slow, Visible_Medium), Accurate);
	fuzzyModule.AddRule(FzAND(Target_Far, Slow, Visible_Brief), Accurate);
}

//-------------------------- PredictFuturePositionOfTarget --------------------
//
//  predicts where the target will be located in the time it takes for a
//  projectile to reach it. This uses a similar logic to the Pursuit steering
//  behavior.
//-----------------------------------------------------------------------------
Vector2D Raven_WeaponSystem::PredictFuturePositionOfTarget()const
{
  double MaxSpeed = GetCurrentWeapon()->GetMaxProjectileSpeed();
  
  //if the target is ahead and facing the agent shoot at its current pos
  Vector2D ToEnemy = m_pOwner->GetTargetBot()->Pos() - m_pOwner->Pos();
 
  //the lookahead time is proportional to the distance between the enemy
  //and the pursuer; and is inversely proportional to the sum of the
  //agent's velocities
  double LookAheadTime = ToEnemy.Length() / 
                        (MaxSpeed + m_pOwner->GetTargetBot()->MaxSpeed());
  
  //return the predicted future position of the enemy
  return m_pOwner->GetTargetBot()->Pos() + 
         m_pOwner->GetTargetBot()->Velocity() * LookAheadTime;
}


//------------------ GetAmmoRemainingForWeapon --------------------------------
//
//  returns the amount of ammo remaining for the specified weapon. Return zero
//  if the weapon is not present
//-----------------------------------------------------------------------------
int Raven_WeaponSystem::GetAmmoRemainingForWeapon(unsigned int weapon_type)
{
  if (m_WeaponMap[weapon_type])
  {
    return m_WeaponMap[weapon_type]->NumRoundsRemaining();
  }

  return 0;
}

//---------------------------- ShootAt ----------------------------------------
//
//  shoots the current weapon at the given position
//-----------------------------------------------------------------------------
void Raven_WeaponSystem::ShootAt(Vector2D pos)const
{
  GetCurrentWeapon()->ShootAt(pos);
}

//-------------------------- RenderCurrentWeapon ------------------------------
//-----------------------------------------------------------------------------
void Raven_WeaponSystem::RenderCurrentWeapon()const
{
  GetCurrentWeapon()->Render();
}

void Raven_WeaponSystem::RenderDesirabilities()const
{
  Vector2D p = m_pOwner->Pos();

  int num = 0;
  
  WeaponMap::const_iterator curWeap;
  for (curWeap=m_WeaponMap.begin(); curWeap != m_WeaponMap.end(); ++curWeap)
  {
    if (curWeap->second) num++;
  }

  int offset = 15 * num;

    for (curWeap=m_WeaponMap.begin(); curWeap != m_WeaponMap.end(); ++curWeap)
    {
      if (curWeap->second)
      {
        double score = curWeap->second->GetLastDesirabilityScore();
        std::string type = GetNameOfType(curWeap->second->GetType());

        gdi->TextAtPos(p.x+10.0, p.y-offset, std::to_string(score) + " " + type);

        offset+=15;
      }
    }
}

//--------------------------- RenderWeaponState -------------------------------
//-----------------------------------------------------------------------------
void Raven_WeaponSystem::RenderWeaponState() 
{
	Vector2D refPos(10, 10);
	Vector2D lineSpace(0, 15);
	Vector2D renderPos;
	std::string line;

	gdi->TextColor(255, 0, 0);
	gdi->TransparentText();

	if(GetCurrentWeapon()->GetType() == type_blaster) gdi->TextColor(0, 0, 255);
	else gdi->TextColor(255, 0, 0);
	renderPos = refPos;
	line = "1 | " + GetNameOfType(type_blaster) + " : " + "infinite";
	gdi->TextAtPos(renderPos, line);

	if (GetCurrentWeapon()->GetType() == type_shotgun) gdi->TextColor(0, 0, 255);
	else gdi->TextColor(255, 0, 0);
	renderPos += lineSpace;
	line = "2 | " + GetNameOfType(type_shotgun) + " : ";
	line.append(std::to_string(GetAmmoRemainingForWeapon(type_shotgun)));
	gdi->TextAtPos(renderPos, line);

	if (GetCurrentWeapon()->GetType() == type_rocket_launcher) gdi->TextColor(0, 0, 255);
	else gdi->TextColor(255, 0, 0);
	renderPos += lineSpace;
	line = "3 | " + GetNameOfType(type_rocket_launcher) + " : ";
	line.append(std::to_string(GetAmmoRemainingForWeapon(type_rocket_launcher)));
	gdi->TextAtPos(renderPos, line);

	if (GetCurrentWeapon()->GetType() == type_rail_gun) gdi->TextColor(0, 0, 255);
	else gdi->TextColor(255, 0, 0);
	renderPos += lineSpace;
	line = "4 | " + GetNameOfType(type_rail_gun) + " : ";
	line.append(std::to_string(GetAmmoRemainingForWeapon(type_rail_gun)));
	gdi->TextAtPos(renderPos, line);

	if (GetCurrentWeapon()->GetType() == type_slowering_gun) gdi->TextColor(0, 0, 255);
	else gdi->TextColor(255, 0, 0);
	renderPos += lineSpace;
	line = "5 | " + GetNameOfType(type_slowering_gun) + " : ";
	line.append(std::to_string(GetAmmoRemainingForWeapon(type_slowering_gun)));
	gdi->TextAtPos(renderPos, line);
}