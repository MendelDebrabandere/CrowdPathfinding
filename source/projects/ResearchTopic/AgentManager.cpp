#include "stdafx.h"
#include "AgentManager.h"

#include "VectorMapManager.h"
#include "projects/Movement/SteeringBehaviors/SteeringAgent.h"

using namespace Elite;

Elite::AgentManager::AgentManager(GridGraph<GridTerrainNode, GraphConnection>* gridPtr)
	:m_pGridGraph{ gridPtr }
{
}

Elite::AgentManager::~AgentManager()
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

void AgentManager::UpdateAgentsAndCreateVectors(float dTime, int destinationIdx, int sizeCell, VectorMapManager* pVectorManager,const HeatmapManager* pHeatmapManager)
{
	for (SteeringAgent* a : m_Agents)
	{
		const int idx{ m_pGridGraph->GetNodeIdxAtWorldPos(a->GetPosition()) };
		if (idx != -1)
			if (destinationIdx == idx)
				a->SetLinearVelocity(m_pGridGraph->GetNodeWorldPos(destinationIdx) - a->GetPosition());
			else
				a->SetLinearVelocity((*pVectorManager->GetVectorMap())[idx] * 500);

		
		const int nodeIdx{ m_pGridGraph->GetNodeIdxAtWorldPos(a->GetPosition()) };

		if ((*pVectorManager->GetVectorMap())[nodeIdx].MagnitudeSquared() <= 0.2f)
			pVectorManager->CreateAndSetVector(nodeIdx, destinationIdx, sizeCell, pHeatmapManager);

		a->Update(dTime);
	}
}

void AgentManager::AddAgent(const Vector2& position)
{
	SteeringAgent* agent{};
	agent = new SteeringAgent(4);
	agent->SetAutoOrient(true);
	agent->SetMaxLinearSpeed(300);
	agent->SetMass(1);
	
	agent->SetPosition(position);

	m_Agents.push_back(agent);
}

void AgentManager::Add30Agents(const Vector2& position)
{
	for (int i{}; i < 30; ++i)
	{
		AddAgent(position + Vector2{float(i), float(i)});
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

