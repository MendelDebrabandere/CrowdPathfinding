#ifndef RESEARCHTOPIC_APPLICATION_H
#define RESEARCHTOPIC_APPLICATION_H

#include "framework/EliteAI/EliteGraphs/EInfluenceMap.h"
#include "framework/EliteInterfaces/EIApp.h"
#include "framework\EliteAI\EliteGraphs\EGridGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphEditor.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphRenderer.h"


class SteeringAgent;
class ISteeringBehavior;

class App_ResearchTopic : public IApp
{
public:
	//Constructor & Destructor
	App_ResearchTopic() = default;
	virtual ~App_ResearchTopic();

	// rule of 5 copypasta
	App_ResearchTopic(const App_ResearchTopic& other) = delete;
	App_ResearchTopic(App_ResearchTopic&& other) = delete;
	App_ResearchTopic& operator=(const App_ResearchTopic& other) = delete;
	App_ResearchTopic& operator=(App_ResearchTopic&& other) = delete;



	//App Functions
	void Start() override;
	void Update(float deltaTime) override;
	void Render(float deltaTime) const override;

private:
	struct DebugSettings
	{
		bool showHeatMap = false;
		bool showNumberHeatMap = false;
		bool showVectorMap = false;
	};

	//Datamembers
	Elite::Vector2 m_StartPosition = Elite::ZeroVector2;
	Elite::Vector2 m_TargetPosition = Elite::ZeroVector2;

	//Grid datamembers
	static constexpr int COLUMNS = 20;
	static constexpr int ROWS = 20;
	unsigned int m_SizeCell = 15;
	Elite::GridGraph<Elite::GridTerrainNode, Elite::GraphConnection>* m_pGridGraph;

	//VectorField datamembers
	int destinationIdx = invalid_node_index;
	std::vector<int> m_HeatMap{};
	std::vector<Elite::Vector2> m_VectorMap{};

	//Editor and Visualisation
	Elite::GraphEditor* m_pGraphEditor{ nullptr };
	Elite::GraphRenderer* m_pGraphRenderer{ nullptr };

	//Debug rendering information
	DebugSettings m_DebugSettings{};

	//Agents
	struct ImGui_Agent
	{
		SteeringAgent* pAgent = nullptr;
		ISteeringBehavior* pBehavior = nullptr;
	};
	std::vector<ImGui_Agent> m_AgentVec{};

	//Walls
	//std::vector

	//Functions
	void MakeGridGraph();
	void UpdateImGui();
	void CalculateHeatMap();
	void UpdateVectorMap();
	Elite::Vector2 CreateVector(int idx) const;
	void DrawVectors() const;
	void AddAgent(const Elite::Vector2& position);
};

#endif
