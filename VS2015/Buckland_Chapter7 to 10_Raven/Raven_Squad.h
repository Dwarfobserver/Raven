#pragma once
#include "Raven_Bot.h"

class Raven_Squad
{

	Raven_Bot* leader;

	std::vector<Raven_Bot*> squad;

	Raven_Bot* target;

public:
	Raven_Squad();
	~Raven_Squad();

	// Utilis� par le leader
	void setTarget();

	// Utilis� par l'escouade
	Raven_Bot* getTarget(){ return target; }

	// ajoute un bot � l'escouade
	void addBot(Raven_Bot);
	// retire 
};

