//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "SteeringBehaviors.h"
#include "../SteeringAgent.h"
#include "../Obstacle.h"
#include "framework\EliteMath\EMatrix2x3.h"

//SEEK
//****
SteeringOutput Seek::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior() && m_DebugRender)
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0,1,0 });

	return steering;
}

//FLEE
//****
SteeringOutput Flee::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{	
	SteeringOutput steering = {};

	steering.LinearVelocity = pAgent->GetPosition() - m_Target.Position;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior() && m_DebugRender)
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0,1,0 });

	return steering;
}

//ARRIVE
//****
SteeringOutput Arrive::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	if ((m_Target.Position - pAgent->GetPosition()).Magnitude() <= 1)
	{
		steering.LinearVelocity = Elite::Vector2{0,0};
		return steering;
	}


	steering.LinearVelocity = m_Target.Position - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior() && m_DebugRender)
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), {0,1,0});

	return steering;
}

//FACE
//****
SteeringOutput Face::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	pAgent->SetAutoOrient(false);

	SteeringOutput steering = {};

	float agentAngle{ pAgent->GetRotation() };

	float diagonal{ sqrtf(powf(m_Target.Position.y - pAgent->GetPosition().y,2) + powf(m_Target.Position.x - pAgent->GetPosition().x, 2))};

	Elite::Vector2 agentPos{ pAgent->GetPosition() };

	float angleFromSinToTarget{ asin((m_Target.Position.y - agentPos.y) / diagonal)   };
	float angleFromCosToTarget{ acos((m_Target.Position.x - agentPos.x) / diagonal)   };

	float angleToTarget{};

	if (angleFromCosToTarget <= 3.1415f / 2)
	{
		angleToTarget = angleFromSinToTarget;
	}
	else
	{
		if (angleFromSinToTarget >= 0)
		{
			angleToTarget = 3.1415f - angleFromSinToTarget;
		}
		else
		{
			angleToTarget = -3.1315f - angleFromSinToTarget;
		}
	}
	
	float angleToDo{ angleToTarget  - agentAngle };

	if (abs(angleToDo) > 3.1415f)
	{
		if (angleToDo < -3.1415f)
			angleToDo += 2 * 3.1415f;
		else
			angleToDo -= 2 * 3.1415f;
	}

	steering.AngularVelocity = angleToDo;
	

	return steering;
}

//WANDER
//****
SteeringOutput Wander::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	float circleRadius{ 5 };
	int maxWanderAngle{ 65 };

	int randAngle = int(Elite::ToRadians(float((rand() % maxWanderAngle) - (rand() % maxWanderAngle))));
	m_WanderAngle += randAngle;

	Elite::Vector2 circleCenter{ pAgent->GetPosition() + pAgent->GetDirection().GetNormalized() * (circleRadius + 2) };
	Elite::Vector2 randomPoint{ circleCenter.x + cosf(m_WanderAngle) * circleRadius , circleCenter.y + sinf(m_WanderAngle) * circleRadius };


	if (pAgent->CanRenderBehavior() && m_DebugRender)
	{
		DEBUGRENDERER2D->DrawCircle(circleCenter, circleRadius, { 0,0,1 }, 0.9f);
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0,1,0 });
		DEBUGRENDERER2D->DrawPoint(randomPoint, 3, { 1,0,0 });
	}

	steering.LinearVelocity = randomPoint - pAgent->GetPosition();
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	

	return steering;
}

//PURSUIT
//****
SteeringOutput Pursuit::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	Elite::Vector2 distance{ m_Target.Position - pAgent->GetPosition() };
	float T{ distance.Magnitude() / pAgent->GetMaxLinearSpeed()};
	Elite::Vector2 direction{ m_Target.Position + m_Target.LinearVelocity * T};

	direction -= pAgent->GetPosition();

	steering.LinearVelocity = direction;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior() && m_DebugRender)
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0,1,0 });
	}

	return steering;
}

//EVADE
//****
SteeringOutput Evade::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};

	Elite::Vector2 toTarget = pAgent->GetPosition() - (m_Target).Position;
	float distanceSquared = toTarget.MagnitudeSquared();

	if (distanceSquared > m_EvadeRadius * m_EvadeRadius)
	{
		steering.IsValid = false;
		return steering;

	}


	Elite::Vector2 distance{ m_Target.Position - pAgent->GetPosition() };
	float T{ distance.Magnitude() / pAgent->GetMaxLinearSpeed() };
	Elite::Vector2 direction{ m_Target.Position + m_Target.LinearVelocity * T };

	direction -= pAgent->GetPosition();

	steering.LinearVelocity = -direction;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior() && m_DebugRender)
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0,1,0 });
	}

	return steering;
}

//HIDE
//****
SteeringOutput Hide::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};
	
	if ((*m_ObstaclePtrs).size() == 0)
	{
		return steering;
	}

	float hideDistance{ 2 };

	float closestDistanceSquared{ FLT_MAX };
	int closestIdx{};

	for (UINT idx{}; idx < (*m_ObstaclePtrs).size(); ++idx)
	{
		Elite::Vector2 distance{ (*m_ObstaclePtrs)[idx]->GetCenter() - pAgent->GetPosition() };

		if (closestDistanceSquared > distance.MagnitudeSquared())
		{
			closestDistanceSquared = distance.MagnitudeSquared();
			closestIdx = idx;
		}
	}

	Elite::Vector2 direction{ (*m_ObstaclePtrs)[closestIdx]->GetCenter() - m_Target.Position };
	float directionMagnitude = direction.Magnitude();
	direction = direction / directionMagnitude;

	direction *= ((*m_ObstaclePtrs)[closestIdx]->GetRadius() + hideDistance);

	Elite::Vector2 directionInWorldView{ (*m_ObstaclePtrs)[closestIdx]->GetCenter() + direction };

	Elite::Vector2 directionFromAgent{ directionInWorldView - pAgent->GetPosition() };

	if (directionFromAgent.Magnitude() <= 1.f)
	{
		directionFromAgent = Elite::Vector2{ 0,0 };
	}

	steering.LinearVelocity = directionFromAgent;
	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();

	if (pAgent->CanRenderBehavior() && m_DebugRender)
	{
		DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0,1,0 });
		DEBUGRENDERER2D->DrawCircle((*m_ObstaclePtrs)[closestIdx]->GetCenter(), (*m_ObstaclePtrs)[closestIdx]->GetRadius() + 1, Elite::Color{ 1,0,0 }, 0.9f);
		DEBUGRENDERER2D->DrawPoint(directionFromAgent - pAgent->GetPosition(), 2, Elite::Color{ 1,0,0 }, 0.9f);
	}

	return steering;
}

//ObstacleAvoidance
//****
SteeringOutput AvoidObstacle::CalculateSteering(float deltaT, SteeringAgent* pAgent)
{
	SteeringOutput steering = {};


	float fleeRadius{ 5 };

	if (pAgent->CanRenderBehavior() && m_DebugRender)
		DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), fleeRadius, {1,0,0}, 0.9f);


	bool hasObstacleInFront{ false };
	for (UINT idx{}; idx < (*m_ObstaclePtrs).size(); ++idx)
	{
		Elite::Vector2 fromObstacleToAgent{ pAgent->GetPosition() - (*m_ObstaclePtrs)[idx]->GetCenter() };
		if (fromObstacleToAgent.MagnitudeSquared() < ((*m_ObstaclePtrs)[idx]->GetRadius() + fleeRadius) * ((*m_ObstaclePtrs)[idx]->GetRadius() + fleeRadius))
		{
			steering.LinearVelocity = pAgent->GetPosition() - (*m_ObstaclePtrs)[idx]->GetCenter();
			if (pAgent->CanRenderBehavior() && m_DebugRender)
				DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0,1,0 });
			hasObstacleInFront = true;
			break;
		}
	}

	if (!hasObstacleInFront)
	{
		float circleRadius{ 5.f };
		int maxWanderAngle{ 65 };

		int randAngle = int(Elite::ToRadians(float((rand() % maxWanderAngle) - (rand() % maxWanderAngle))));
		m_WanderAngle += randAngle;

		Elite::Vector2 circleCenter{ pAgent->GetPosition() + pAgent->GetDirection().GetNormalized() * (circleRadius + 2) };
		Elite::Vector2 randomPoint{ circleCenter.x + cosf(m_WanderAngle) * circleRadius , circleCenter.y + sinf(m_WanderAngle) * circleRadius };


		if (pAgent->CanRenderBehavior() && m_DebugRender)
		{
			DEBUGRENDERER2D->DrawCircle(circleCenter, circleRadius, { 0,0,1 }, 0.9f);
			DEBUGRENDERER2D->DrawDirection(pAgent->GetPosition(), steering.LinearVelocity, steering.LinearVelocity.Magnitude(), { 0,1,0 });
			DEBUGRENDERER2D->DrawPoint(randomPoint, 3, { 1,0,0 });
		}

		Elite::Vector2 agentPos{ pAgent->GetPosition() };
		steering.LinearVelocity = randomPoint - agentPos;
	}


	steering.LinearVelocity.Normalize();
	steering.LinearVelocity *= pAgent->GetMaxLinearSpeed();


	return steering;

}