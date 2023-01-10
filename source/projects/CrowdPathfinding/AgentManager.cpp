#include "stdafx.h"
#include "AgentManager.h"

#include "projects/Movement/SteeringBehaviors/SteeringAgent.h"

using namespace Elite;

//Agents
AgentManager::AgentManager()
{
	for (int i{}; i < 10; ++i)
	{
		SteeringAgent* agent{};
		agent = new SteeringAgent(0.2f);
		agent->SetAutoOrient(true);
		agent->SetMaxLinearSpeed(50);
		agent->SetMass(1);

		m_Agents.push_back(agent);
	}
	m_Agents[0]->SetPosition(Vector2{ 52, 7 });
	m_Agents[1]->SetPosition(Vector2{ 57, 7 });
	m_Agents[2]->SetPosition(Vector2{ 62, 7 });
	m_Agents[3]->SetPosition(Vector2{ 67, 7 });
	m_Agents[4]->SetPosition(Vector2{ 72, 7 });
	m_Agents[5]->SetPosition(Vector2{ 47, 7 });
	m_Agents[6]->SetPosition(Vector2{ 87, 52 });
	m_Agents[7]->SetPosition(Vector2{ 87, 57 });
	m_Agents[8]->SetPosition(Vector2{ 83, 67 });
	m_Agents[9]->SetPosition(Vector2{ 17, 74 });

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

//void AgentManager::UpdateAgentsAndCreateVectors(float dTime, int destinationIdx, int sizeCell, VectorMapManager* pVectorManager,const HeatmapManager* pHeatmapManager)
//{
//	for (SteeringAgent* a : m_Agents)
//	{
//		const int idx{ m_pGridGraph->GetNodeIdxAtWorldPos(a->GetPosition()) };
//		if (idx != -1)
//			if (destinationIdx == idx)
//				a->SetLinearVelocity(m_pGridGraph->GetNodeWorldPos(destinationIdx) - a->GetPosition());
//			else
//				a->SetLinearVelocity((*pVectorManager->GetVectorMap())[idx] * 100);
//
//		
//		const int nodeIdx{ m_pGridGraph->GetNodeIdxAtWorldPos(a->GetPosition()) };
//
//		if ((*pVectorManager->GetVectorMap())[nodeIdx].MagnitudeSquared() <= 0.2f)
//			pVectorManager->CreateAndSetVector(nodeIdx, destinationIdx, sizeCell, pHeatmapManager);
//
//		a->Update(dTime);
//	}
//}
//
//void AgentManager::AddAgent(const Vector2& position)
//{
//	SteeringAgent* agent{};
//	agent = new SteeringAgent(4);
//	agent->SetAutoOrient(true);
//	agent->SetMaxLinearSpeed(50);
//	agent->SetMass(10000);
//	
//	agent->SetPosition(position);
//
//	m_Agents.push_back(agent);
//}
//
//void AgentManager::Add30Agents(const Vector2& position)
//{
//	for (int i{}; i < 30; ++i)
//	{
//		AddAgent(position + Vector2{float(i), float(i)});
//	}
//}
//
//void AgentManager::ClearAllAgents()
//{
//	for (auto a : m_Agents)
//	{
//		SAFE_DELETE(a);
//	}
//	m_Agents.clear();
//}

