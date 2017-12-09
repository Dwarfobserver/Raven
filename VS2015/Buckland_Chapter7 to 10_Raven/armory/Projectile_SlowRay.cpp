#include "Projectile_SlowRay.h"
#include "../lua/Raven_Scriptor.h"
#include "misc/cgdi.h"
#include "../Raven_Bot.h"
#include "../Raven_Game.h"
#include "../constants.h"
#include "2d/WallIntersectionTests.h"
#include "../Raven_Map.h"

#include "../Raven_Messages.h"
#include "Messaging/MessageDispatcher.h"


//-------------------------- ctor ---------------------------------------------
//-----------------------------------------------------------------------------
SlowRay::SlowRay(Raven_Bot* shooter, Vector2D target) :

	Raven_Projectile(target,
		shooter->GetWorld(),
		shooter->ID(),
		shooter->Pos(),
		shooter->Facing(),
		script->GetInt("SlowRay_Damage"),
		script->GetInt("SlowRay_SpeedDecreasePercentage"),
		script->GetDouble("SlowRay_Scale"),
		script->GetDouble("SlowRay_MaxForce"),
		script->GetDouble("SlowRay_Mass"),
		script->GetDouble("SlowRay_MaxForce"))
{
	assert(target != Vector2D());
}


//------------------------------ Update ---------------------------------------
//-----------------------------------------------------------------------------
void SlowRay::Update()
{
	if (!m_bImpacted)
	{
		m_vVelocity = MaxSpeed() * Heading();

		//make sure vehicle does not exceed maximum velocity
		m_vVelocity.Truncate(m_dMaxSpeed);

		//update the position
		m_vPosition += m_vVelocity;


		//if the projectile has reached the target position or it hits an entity
		//or wall it should explode/inflict damage/whatever and then mark itself
		//as dead


		//test to see if the line segment connecting the slow ray's current position
		//and previous position intersects with any bots.
		Raven_Bot* hit = GetClosestIntersectingBot(m_vPosition - m_vVelocity,
			m_vPosition);

		//if hit
		if (hit)
		{
			m_bImpacted = true;
			m_bDead = true;

			//send a message to the bot to let it know it's been hit, and who the
			//shot came from
			Dispatcher->DispatchMsg(SEND_MSG_IMMEDIATELY,
				m_iShooterID,
				hit->ID(),
				Msg_SlowDown,
				(void*)&m_iSloweringInflected);
		}

		//test for impact with a wall
		double dist;
		if (FindClosestPointOfIntersectionWithWalls(m_vPosition - m_vVelocity,
			m_vPosition,
			dist,
			m_vImpactPoint,
			m_pWorld->GetMap()->GetWalls()))
		{
			m_bDead = true;
			m_bImpacted = true;

			m_vPosition = m_vImpactPoint;

			return;
		}
	}
}


//-------------------------- Render -------------------------------------------
//-----------------------------------------------------------------------------
void SlowRay::Render()
{
	gdi->ThickGreenPen();
	gdi->Line(Pos(), Pos() - Velocity());
}