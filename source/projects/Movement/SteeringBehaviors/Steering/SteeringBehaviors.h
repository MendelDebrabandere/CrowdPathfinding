/*=============================================================================*/
// Copyright 2021-2022 Elite Engine
// Authors: Matthieu Delaere, Thomas Goussaert
/*=============================================================================*/
// SteeringBehaviors.h: SteeringBehaviors interface and different implementations
/*=============================================================================*/
#ifndef ELITE_STEERINGBEHAVIORS
#define ELITE_STEERINGBEHAVIORS

//-----------------------------------------------------------------
// Includes & Forward Declarations
//-----------------------------------------------------------------
#include "../SteeringHelpers.h"
class SteeringAgent;
class Obstacle;

#pragma region **ISTEERINGBEHAVIOR** (BASE)
class ISteeringBehavior
{
public:
	ISteeringBehavior() = default;
	virtual ~ISteeringBehavior() = default;

	virtual SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) = 0;

	//Seek Functions
	void SetTarget(const TargetData& target) { m_Target = target; }

	void SetDebugRender(bool newValue) { m_DebugRender = newValue; }

	template<class T, typename std::enable_if<std::is_base_of<ISteeringBehavior, T>::value>::type* = nullptr>
	T* As()
	{ return static_cast<T*>(this); }

protected:
	TargetData m_Target;
	bool m_DebugRender = true;
};
#pragma endregion

///////////////////////////////////////
//SEEK
//****
class Seek : public ISteeringBehavior
{
public:
	Seek() = default;
	virtual ~Seek() = default;

	//Seek Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};


///////////////////////////////////////
//FLEE
//****
class Flee final: public ISteeringBehavior
{
public:
	Flee() = default;
	virtual ~Flee() = default;
	
	//Flee Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};


///////////////////////////////////////
//ARRIVE
//****
class Arrive final: public ISteeringBehavior
{
public:
	Arrive() = default;
	virtual ~Arrive() = default;

	//Arrive Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};

///////////////////////////////////////
//FACE
//****
class Face final: public ISteeringBehavior
{
public:
	Face() = default;
	virtual ~Face() = default;

	//Face Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
};

///////////////////////////////////////
//WANDER
//****
class Wander final: public ISteeringBehavior
{
public:
	Wander() = default;
	virtual ~Wander() = default;

	//Wander Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;
private:
	float m_WanderAngle = 0.f;
};

///////////////////////////////////////
//PURSUIT
//****
class Pursuit final: public ISteeringBehavior
{
public:
	Pursuit() = default;
	virtual ~Pursuit() = default;

	//Pursuit Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

};

///////////////////////////////////////
//EVADE
//****
class Evade final: public ISteeringBehavior
{
public:
	Evade() = default;
	virtual ~Evade() = default;

	//Evade Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

	void SetEvadeRadius(float evadeRadius) { m_EvadeRadius = evadeRadius; }

private:
	float m_EvadeRadius = 20.f;

};

///////////////////////////////////////
//HIDE
//****
class Hide final : public ISteeringBehavior
{
public:
	Hide(std::vector<Obstacle*>* obstaclePtrs) :m_ObstaclePtrs{ obstaclePtrs } {};
	virtual ~Hide() = default;

	//Evade Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	std::vector<Obstacle*>* m_ObstaclePtrs;

};

///////////////////////////////////////
//ObstacleAvoidance
//****
class AvoidObstacle final : public ISteeringBehavior
{
public:
	AvoidObstacle(std::vector<Obstacle*>* obstaclePtrs) :m_ObstaclePtrs{ obstaclePtrs } {};
	virtual ~AvoidObstacle() = default;

	//Evade Behaviour
	SteeringOutput CalculateSteering(float deltaT, SteeringAgent* pAgent) override;

private:
	float m_WanderAngle = 0.f;
	std::vector<Obstacle*>* m_ObstaclePtrs;
};
#endif


