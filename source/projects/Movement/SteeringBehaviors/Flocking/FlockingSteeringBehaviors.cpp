#include "stdafx.h"
#include "FlockingSteeringBehaviors.h"
#include "Flock.h"
#include "../SteeringAgent.h"
#include "../SteeringHelpers.h"


//*******************
//COHESION (FLOCKING)
SteeringOutput Cohesion::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	Elite::Vector2 averagePos{ m_pFlock->GetAverageNeighborPos() };

	SteeringOutput steering = {};

	steering.LinearVelocity = averagePos - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior() && m_DebugRender)
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0,1,0 });

	return steering;
}

//*********************
//SEPARATION (FLOCKING)
SteeringOutput Separation::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	std::vector<SteeringAgent*> neighbors{ m_pFlock->GetNeighbors() };

	Elite::Vector2 direction{};

	for (auto pNeighAgent : neighbors)
	{
		Elite::Vector2 addingVector{ pAgent->GetPosition() - pNeighAgent->GetPosition() };

		if (!addingVector.x == 0.f)
		{
			addingVector.x = 1 / addingVector.x;
		}
		if (!addingVector.y == 0.f)
		{
			addingVector.y = 1 / addingVector.y;
		}

		direction += addingVector;
	}


	SteeringOutput steering = {};

	steering.LinearVelocity = direction;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior() && m_DebugRender)
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0,1,0 });

	return steering;
}

//*************************
//VELOCITY MATCH (FLOCKING)
SteeringOutput VelocityMatch::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	Elite::Vector2 averageVel{ m_pFlock->GetAverageNeighborVelocity() };

	SteeringOutput steering = {};

	steering.LinearVelocity = averageVel;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior() && m_DebugRender)
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0,1,0 });

	return steering;
}
