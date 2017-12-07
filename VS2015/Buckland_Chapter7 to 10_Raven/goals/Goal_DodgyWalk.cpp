#include "Goal_DodgyWalk.h"
#include "..\Raven_Bot.h"
#include "Raven_Goal_Types.h"
#include "..\Raven_SteeringBehaviors.h"
#include "time/CrudeTimer.h"
#include "..\constants.h"
#include "../navigation/Raven_PathPlanner.h"
#include "misc/cgdi.h"
#include "../lua/Raven_Scriptor.h"


#include "debug/DebugConsole.h"



//---------------------------- ctor -------------------------------------------
//-----------------------------------------------------------------------------
Goal_DodgyWalk::Goal_DodgyWalk(Raven_Bot* pBot,
	PathEdge   edge,
	bool       LastEdge) :

	Goal<Raven_Bot>(pBot, goal_godgywalk),
	m_Edge(edge),
	m_dTimeExpected(0.0),
	m_bLastEdgeInPath(LastEdge)

{}


//---------------------------- Activate -------------------------------------
//-----------------------------------------------------------------------------  
void Goal_DodgyWalk::Activate()
{
	m_iStatus = active;

	//the edge behavior flag may specify a type of movement that necessitates a 
	//change in the bot's max possible speed as it follows this edge
	switch (m_Edge.Behavior())
	{
	case NavGraphEdge::swim:
	{
		m_pOwner->SetMaxSpeed(script->GetDouble("Bot_MaxSwimmingSpeed"));
	}

	break;

	case NavGraphEdge::crawl:
	{
		m_pOwner->SetMaxSpeed(script->GetDouble("Bot_MaxCrawlingSpeed"));
	}

	break;
	}


	//record the time the bot starts this goal
	m_dStartTime = Clock->GetCurrentTime();

	// Et là on tente de straffer ----------

	static const double StepDistance = m_pOwner->BRadius() * 2;

	cheminChoisi = m_Edge.Destination();
	Vector2D direction = (cheminChoisi - m_pOwner->Pos()).Perp();

	direction.Normalize();

	//Vector2D Gauche  = cheminChoisi - direction * StepDistance - direction * m_pOwner->BRadius(); 
	//Vector2D Droite  = cheminChoisi + direction * StepDistance + direction * m_pOwner->BRadius();

	Vector2D Gauche = cheminChoisi - direction * RandInt(5, 20);
	Vector2D Droite = cheminChoisi + direction * RandInt(5, 20);

	// On rend clockwise random (stratégie d'esquive un peu brouillonne/paniquée
	m_bClockwise = rand() % 2;


	

	// Pour alterner on fait appel à clockwise

	if (m_bClockwise) // On commence par la droite
	{
		// On tente à droite

		if (m_pOwner->canWalkTo(Droite))
		{


			cheminChoisi = Droite;
			m_bClockwise = !m_bClockwise;
		}
		// Si ça marche pas
		//  On tente à gauche
		else if (m_pOwner->canWalkTo(Gauche))
		{


			cheminChoisi = Gauche;
			m_bClockwise = !m_bClockwise;
		}
		// Si ça marche pas
		//  On va tout droit
		else {


			cheminChoisi = m_Edge.Destination();
			m_bClockwise = !m_bClockwise;
		}
	}
	else {	// On commence par la gauche
			//  On tente à gauche
		if (m_pOwner->canWalkTo(Gauche))
		{


			cheminChoisi = Gauche;
			m_bClockwise = !m_bClockwise;
		}
		// Si ça marche pas
		// On tente à droite
		else if (m_pOwner->canWalkTo(Droite))
		{


			cheminChoisi = Droite;
			m_bClockwise = !m_bClockwise;
		}
		// Si ça marche pas
		//  On va tout droit
		else {
			cheminChoisi = m_Edge.Destination();
			m_bClockwise = !m_bClockwise;
		}
	}


	//calculate the expected time required to reach the this waypoint. This value
	//is used to determine if the bot becomes stuck 
	//---
	//m_dTimeExpected = m_pOwner->CalculateTimeToReachPosition(m_Edge.Destination());
	//---
	m_dTimeExpected = m_pOwner->CalculateTimeToReachPosition(cheminChoisi);

	//factor in a margin of error for any reactive behavior
	static const double MarginOfError = 2.0;

	m_dTimeExpected += MarginOfError;


	//set the steering target
	m_pOwner->GetSteering()->SetTarget(cheminChoisi);

	//Set the appropriate steering behavior. If this is the last edge in the path
	//the bot should arrive at the position it points to, else it should seek
	if (m_bLastEdgeInPath)
	{
		m_pOwner->GetSteering()->ArriveOn();
	}

	else
	{
		m_pOwner->GetSteering()->SeekOn();
	}
}



//------------------------------ Process --------------------------------------
//-----------------------------------------------------------------------------
int Goal_DodgyWalk::Process()
{
	//if status is inactive, call Activate()
	ActivateIfInactive();

	//if the bot has become stuck return failure
	if (isStuck())
	{
		m_iStatus = failed;
	}

	//if the bot has reached the end of the edge return completed
	else
	{
		if (m_pOwner->isAtPosition(cheminChoisi))
		{
			m_iStatus = completed;
		}
	}

	return m_iStatus;
}

//--------------------------- isBotStuck --------------------------------------
//
//  returns true if the bot has taken longer than expected to reach the 
//  currently active waypoint
//-----------------------------------------------------------------------------
bool Goal_DodgyWalk::isStuck()const
{
	double TimeTaken = Clock->GetCurrentTime() - m_dStartTime;

	if (TimeTaken > m_dTimeExpected)
	{
		debug_con << "BOT " << m_pOwner->ID() << " IS STUCK!!" << "";

		return true;
	}

	return false;
}


//---------------------------- Terminate --------------------------------------
//-----------------------------------------------------------------------------
void Goal_DodgyWalk::Terminate()
{
	//turn off steering behaviors.
	m_pOwner->GetSteering()->SeekOff();
	m_pOwner->GetSteering()->ArriveOff();

	//return max speed back to normal
	m_pOwner->SetMaxSpeed(script->GetDouble("Bot_MaxSpeed"));
}

//----------------------------- Render ----------------------------------------
//-----------------------------------------------------------------------------
void Goal_DodgyWalk::Render()
{
	if (m_iStatus == active)
	{
		gdi->BluePen();
		gdi->Line(m_pOwner->Pos(), m_Edge.Destination());
		gdi->GreenBrush();
		gdi->BlackPen();
		gdi->Circle(m_Edge.Destination(), 3);
	}
}

