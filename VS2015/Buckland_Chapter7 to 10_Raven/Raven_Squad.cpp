#include "Raven_Squad.h"
#include "debug/DebugConsole.h"


std::list<Raven_Squad*> Raven_Squad::squadList;

Raven_Squad::Raven_Squad(Raven_Bot* leader, int color):	color(color)
{
	squad.push_front(leader);
	leader->squad = this;
}


Raven_Squad::~Raven_Squad()
{
}

void Raven_Squad::updateTarget()
{
	Raven_Bot* target = getLeader()->GetTargetSys()->GetTarget();
	for (Raven_Bot* membre : squad)
	{
		membre->GetTargetSys()->SetTarget(target);
	}
}

void Raven_Squad::addToSquad(Raven_Bot* bot, int color)
{
	Raven_Squad* squad = nullptr;
	for (Raven_Squad* a : Raven_Squad::squadList)
	{
		if (a->color == color)
		{
			squad = a;
		}
	}

	if (squad == nullptr)
	{
		squad = new Raven_Squad(bot, color);
		squadList.push_back(squad);
	}
	else
	{
		squad->addBot(bot);
	}
}


