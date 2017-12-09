#ifndef SLOWRAY_H
#define SLOWRAY_H
#pragma warning (disable:4786)

#include "Raven_Projectile.h"

class Raven_Bot;



class SlowRay : public Raven_Projectile
{
private:

	//tests the trajectory of the shell for an impact
	void TestForImpact();

public:

	SlowRay(Raven_Bot* shooter, Vector2D target);

	void Render();

	void Update();

};


#endif