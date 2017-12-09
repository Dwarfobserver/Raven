#ifndef SLOWERINGGUN_H
#define SLOWERINGGUN_H
#pragma warning (disable:4786)
//-----------------------------------------------------------------------------
//
//  Name:   Weapon_SloweringGun.h
//
//  Author: Mat Buckland (www.ai-junkie.com)
//
//  Desc:   class to implement a shot gun
//-----------------------------------------------------------------------------
#include "Raven_Weapon.h"

class  Raven_Bot;



class SlowingGun : public Raven_Weapon
{
private:

	void     InitializeFuzzyModule();

public:

	SlowingGun(Raven_Bot* owner);

	void  Render();

	void  ShootAt(Vector2D pos);

	double GetDesirability(double DistToTarget);
};



#endif