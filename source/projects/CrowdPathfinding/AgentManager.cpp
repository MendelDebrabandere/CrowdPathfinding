#include "stdafx.h"
#include "AgentManager.h"

#include "Sector.h"
#include "projects/Movement/SteeringBehaviors/SteeringAgent.h"

using namespace Elite;

//Agents
AgentManager::AgentManager()
{
};


AgentManager::~AgentManager()
{
	for (auto a : m_Agents)
	{
		SAFE_DELETE(a);
	}
}

int AgentManager::GetAmountOfAgents() const
{
	return m_Agents.size();
}

void AgentManager::RenderAgents(float dTime) const
{
	for (SteeringAgent* a : m_Agents)
	{
		a->Render(dTime);
	}
}

const std::vector<SteeringAgent*>* AgentManager::GetAgents() const
{
	return &m_Agents;
}

void AgentManager::UpdateAgents(float dTime, const std::vector<Sector*>* pSectors)
{
	for (SteeringAgent* a : m_Agents)
	{
		Vector2 pos{ a->GetPosition() };
		Vector2 vector{};
		for (Sector* pSector : *pSectors)
		{
			vector = pSector->GetFlowVector(pos);
			if (vector != Vector2{ 0,0 })
				break;
		}


		a->SetLinearVelocity(vector * 15);

		a->Update(dTime);
	}
}

void AgentManager::AddAgent(const Vector2& position)
{
	SteeringAgent* agent{};
	agent = new SteeringAgent(0.3f);
	agent->SetAutoOrient(true);
	agent->SetMaxLinearSpeed(50);
	agent->SetMass(1);
	
	agent->SetPosition(position);

	m_Agents.push_back(agent);
}

void AgentManager::Add30Agents(const Vector2& position)
{
	for (int i{}; i < 30; ++i)
	{
		AddAgent(position + Vector2{float(i) / 5, float(i) / 5 });
	}
}

void AgentManager::ClearAllAgents()
{
	for (auto a : m_Agents)
	{
		SAFE_DELETE(a);
	}
	m_Agents.clear();
}

