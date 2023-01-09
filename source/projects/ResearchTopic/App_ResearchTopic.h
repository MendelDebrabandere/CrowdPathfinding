#ifndef RESEARCHTOPIC_APPLICATION_H
#define RESEARCHTOPIC_APPLICATION_H

#include "framework/EliteAI/EliteGraphs/EInfluenceMap.h"
#include "framework/EliteInterfaces/EIApp.h"
#include "framework\EliteAI\EliteGraphs\EGridGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphEditor.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphUtilities\EGraphRenderer.h"


class SteeringAgent;

namespace Elite
{
	class HeatmapManager;
	class VectorMapManager;
	class AgentManager;
	class LevelManager;

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
			bool showVectorMap = false;
		};

		//Datamembers
		Vector2 m_StartPosition = ZeroVector2;
		Vector2 m_TargetPosition = ZeroVector2;

		//Grid datamembers
		int COLUMNS;
		int ROWS;
		unsigned int m_SizeCell = 15;
		GridGraph<GridTerrainNode, GraphConnection>* m_pGridGraph;

		//VectorField datamembers
		int destinationIdx = invalid_node_index;
		HeatmapManager* m_pHeatMap{};
		VectorMapManager* m_pVectorMap{};

		//Editor and Visualisation
		GraphEditor* m_pGraphEditor{ nullptr };
		GraphRenderer* m_pGraphRenderer{ nullptr };

		//Debug rendering information
		DebugSettings m_DebugSettings{};

		//Agents
		AgentManager* m_pAgents{};

		//Level
		LevelManager* m_pLevel{};

		//Functions
		void MakeGridGraphAndLevel();
		void UpdateImGui();
		void DrawVectors() const;
		void HandleInput();
		void ParseMapData();
		void MakeSmallTestMap();
	};
}

#endif
