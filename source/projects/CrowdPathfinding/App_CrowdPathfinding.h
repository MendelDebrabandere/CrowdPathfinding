#ifndef CROWDPATHFINDING_APPLICATION_H
#define CROWDPATHFINDING_APPLICATION_H

#include "framework/EliteInterfaces/EIApp.h"


class Portal;


class AgentManager;
class Sector;

class App_CrowdPathfinding: public IApp
{
public:
	//Constructor & Destructor
	App_CrowdPathfinding() = default;
	virtual ~App_CrowdPathfinding();

	// rule of 5 copypasta
	App_CrowdPathfinding(const App_CrowdPathfinding& other) = delete;
	App_CrowdPathfinding(App_CrowdPathfinding&& other) = delete;
	App_CrowdPathfinding& operator=(const App_CrowdPathfinding& other) = delete;
	App_CrowdPathfinding& operator=(App_CrowdPathfinding&& other) = delete;

	//App Functions
	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;

private:
	static constexpr uint8 s_Size{ 10 };

	struct DebugSettings
	{
		bool showSectorBorders = false;
		bool showSectorCells = false;
		bool showPortals = false;
		bool showSectorsToCalc = false;
	};
	DebugSettings m_DebugSettings{};


	std::vector<Sector*> m_SectorPtrs{};
	Elite::Vector2 m_Destination{};

	//Agents
	AgentManager* m_pAgentManager{};

	//Path calc
	std::vector<Sector*> m_SectorsToCalc{};


	void HandleInput();
	void UpdateImGui();
	std::vector<uint8>* ParseMapDataForSectors(int idx);
	void MakeLevel();
	void MakePortals();

	void DoPathCalculations();
	void DoAstarAndAddToVec(Sector* pStartNode);
	float GetHeuristicCost(const Sector* pStartNode) const;

	void GenerateFlowFields();

};

struct NodeRecord
{
	Sector* pNode = nullptr;
	Portal* pConnection = nullptr;
	float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
	float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

	bool operator==(const NodeRecord& other) const
	{
		return pNode == other.pNode
			&& pConnection == other.pConnection
			&& costSoFar == other.costSoFar
			&& estimatedTotalCost == other.estimatedTotalCost;
	};

	bool operator<(const NodeRecord& other) const
	{
		return estimatedTotalCost < other.estimatedTotalCost;
	};
};
#endif
