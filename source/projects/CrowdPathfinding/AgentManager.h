#pragma once

class Sector;
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

	void UpdateAgents(float dTime, const std::vector<Sector*>* pSectors);
	void AddAgent(const Elite::Vector2& position);
	void Add30Agents(const Elite::Vector2& position);
	void ClearAllAgents();

private:
	std::vector<SteeringAgent*> m_Agents{};
};

