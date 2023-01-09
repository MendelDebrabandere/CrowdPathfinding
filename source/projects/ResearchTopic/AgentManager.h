#pragma once
#include "framework/EliteAI/EliteGraphs/EGridGraph.h"

class SteeringAgent;

namespace Elite
{
	class VectorMapManager;
	class HeatmapManager;

	class AgentManager final
	{
	public:
		//Constructor & Destructor
		AgentManager(GridGraph<GridTerrainNode, GraphConnection>* gridPtr);
		~AgentManager();

		// rule of 5 copypasta
		AgentManager(const AgentManager& other) = delete;
		AgentManager(AgentManager&& other) = delete;
		AgentManager& operator=(const AgentManager& other) = delete;
		AgentManager& operator=(AgentManager&& other) = delete;

		int GetAmountOfAgents() const;
		void RenderAgents(float dTime) const;

		void UpdateAgentsAndCreateVectors(float dTime, int destinationIdx, int sizeCell, VectorMapManager* pVectorManager, const HeatmapManager* pHeatmapManager);
		void AddAgent(const Vector2& position);
		void Add30Agents(const Vector2& position);
		void ClearAllAgents();

	private:
		std::vector<SteeringAgent*> m_Agents{};
		GridGraph<GridTerrainNode, GraphConnection>* m_pGridGraph{};
	};
}

