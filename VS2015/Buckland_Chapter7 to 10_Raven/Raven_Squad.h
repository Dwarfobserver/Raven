#pragma once
#include "Raven_Bot.h"

class Raven_Squad
{
	Raven_Bot* leader;
	Raven_Bot* target;
	std::list<Raven_Bot*> squad;

public:
	Raven_Squad(Raven_Bot* leader);
	~Raven_Squad();

	Raven_Bot* getLeader() { return leader; }
	void setLeader(Raven_Bot* a) { leader = a; }

	Raven_Bot* getTarget() { return target; }
	void setTarget(Raven_Bot* a) { target = a; }

	void addBot(Raven_Bot* m) { squad.push_back(m); m->squad = this; }
	void removeBot(Raven_Bot* m) { squad.remove(m); }

	void updateTarget();
};

