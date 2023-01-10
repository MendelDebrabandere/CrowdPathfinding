#pragma once

class SteeringAgent;

class AgentManager final
{
public:
	//Constructor & Destructor
	AgentManager();
	~AgentManager();

	// rule of 5 copypasta
	AgentManager(const AgentManager& other) = delete;
	AgentManager(AgentManager&& other) = delete;
	AgentManager& operator=(const AgentManager& other) = delete;
	AgentManager& operator=(AgentManager&& other) = delete;

	int GetAmountOfAgents() const;
	void RenderAgents(float dTime) const;
	const std::vector<SteeringAgent*>* GetAgents() const;

	//void UpdateAgentsAndCreateVectors(float dTime, int destinationIdx, int sizeCell, VectorMapManager* pVectorManager, const HeatmapManager* pHeatmapManager);
	//void AddAgent(const Vector2& position);
	//void Add30Agents(const Vector2& position);
	//void ClearAllAgents();

private:
	std::vector<SteeringAgent*> m_Agents{};
};

