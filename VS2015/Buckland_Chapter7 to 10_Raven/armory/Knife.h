#pragma once
#include "Raven_Weapon.h"

class Raven_Bot;

class Knife :
	public Raven_Weapon
{
private:
	void  InitializeFuzzyModule();

	double _maxDist;
	int _dmg;

public:

	Knife(Raven_Bot* owner);

	void Render();

	void  ShootAt(Vector2D pos);

	double GetDesirability(double DistToTarget);

};

