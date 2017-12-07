#pragma once
#include "Raven_Bot.h"

class Raven_Squad
{

	Raven_Bot* leader;

	std::list<Raven_Bot*> squad;

	Raven_Bot* target;

public:
	Raven_Squad(Raven_Bot* leader);
	~Raven_Squad();

	void setLeader(Raven_Bot* leader) { this->leader = leader; }
	Raven_Bot* getLeader() { return leader; }

	void setTarget(Raven_Bot* target) { this->target = target; }
	Raven_Bot* getTarget(){ return target; }

	void addBot(Raven_Bot* member) { squad.push_back(member); member->squad = this; }
	void removeBot(Raven_Bot* member) { squad.pop_back(); member->squad = nullptr; }

	void updateTarget();
};

