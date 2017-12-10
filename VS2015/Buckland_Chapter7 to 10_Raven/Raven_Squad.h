#pragma once
#include "Raven_Bot.h"

class Raven_Squad
{
	Raven_Bot* target;
	std::list<Raven_Bot*> squad;

public:
	Raven_Squad(Raven_Bot* leader, int color);
	~Raven_Squad();

	int color;

	Raven_Bot* getLeader() { return squad.front(); }
	void setLeader(Raven_Bot* a) { squad.push_front(a); }

	Raven_Bot* getTarget() { return target; }
	void setTarget(Raven_Bot* a) { target = a; }

	int getSquadSize() { return squad.size(); }
	void addBot(Raven_Bot* m) { squad.push_front(m); m->squad = this; }
	void removeBot(Raven_Bot* m) { squad.remove(m); }

	void updateTarget();

	static std::list<Raven_Squad*> squadList;
	static void addToSquad(Raven_Bot* bot, int color);
};

