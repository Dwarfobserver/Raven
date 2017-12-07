#include "Raven_Squad.h"



Raven_Squad::Raven_Squad(Raven_Bot* leader):leader(leader)
{
	leader->squad = this;
}


Raven_Squad::~Raven_Squad()
{
}

void Raven_Squad::updateTarget()
{
	Raven_Bot* target = leader->GetTargetSys()->GetTarget();
	for (Raven_Bot* membre : squad)
	{
		membre->GetTargetSys()->SetTarget(target);
	}
}
