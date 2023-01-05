//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_ResearchTopic.h"

using namespace Elite;

//Destructor
App_ResearchTopic::~App_ResearchTopic()
{
	SAFE_DELETE(m_pGridGraph);
	SAFE_DELETE(m_pGraphRenderer);
	SAFE_DELETE(m_pGraphEditor);
}

//Functions
void App_ResearchTopic::Start()
{
	m_pGraphEditor = new GraphEditor();
	m_pGraphRenderer = new GraphRenderer();
	//Set Camera
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(39.0f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(73.0f, 35.0f));

	//Create Graph
	MakeGridGraph();

	m_HeatMap.resize(COLUMNS * ROWS);

	//Setup default start path
	destinationIdx = 88;
	CalculateHeatMap();
}

void App_ResearchTopic::Update(float deltaTime)
{
	UNREFERENCED_PARAMETER(deltaTime);

	//INPUT
	bool const middleMousePressed = INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eMiddle);
	if (middleMousePressed)
	{
		MouseData mouseData = { INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eMiddle) };
		Elite::Vector2 mousePos = DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ (float)mouseData.X, (float)mouseData.Y });

		//Find closest node to click pos
		int closestNode = m_pGridGraph->GetNodeIdxAtWorldPos(mousePos);
		destinationIdx = closestNode;
		CalculateHeatMap();
	}
	//IMGUI
	UpdateImGui();

	//UPDATE/CHECK GRID HAS CHANGED
	if (m_pGraphEditor->UpdateGraph(m_pGridGraph))
	{
		CalculateHeatMap();
	}
}

void App_ResearchTopic::Render(float deltaTime) const
{
	UNREFERENCED_PARAMETER(deltaTime);
	//Render grid
	m_pGraphRenderer->RenderGraph(m_pGridGraph, true, m_DebugSettings.showNumberHeatMap, false, false, m_HeatMap);

	//Render end node on top if applicable
	if (destinationIdx != invalid_node_index)
	{
		m_pGraphRenderer->HighlightNodes(m_pGridGraph, { m_pGridGraph->GetNode(destinationIdx) }, END_NODE_COLOR);
	}

	if (m_DebugSettings.showHeatMap == true)
	{
		int furthestIdx{-1};
		for (int distance :m_HeatMap)
		{
			if (distance > furthestIdx && distance != INT_MAX)
				furthestIdx = distance;
		}
		for (int index{}; index < m_HeatMap.size(); ++index)
		{
			m_pGraphRenderer->HighlightNodes(m_pGridGraph, { m_pGridGraph->GetNode(index) }, Elite::Color{ 1 - float(m_HeatMap[index]) / furthestIdx, 0, 0 });
		}
	}
}

void App_ResearchTopic::MakeGridGraph()
{
	m_pGridGraph = new GridGraph<GridTerrainNode, GraphConnection>(COLUMNS, ROWS, m_SizeCell, false, false, 1.f, 1.5f);

	//Setup default terrain
	m_pGridGraph->GetNode(86)->SetTerrainType(TerrainType::Water);
	m_pGridGraph->GetNode(66)->SetTerrainType(TerrainType::Water);
	m_pGridGraph->GetNode(67)->SetTerrainType(TerrainType::Water);
	m_pGridGraph->GetNode(47)->SetTerrainType(TerrainType::Water);
	m_pGridGraph->RemoveConnectionsToAdjacentNodes(86);
	m_pGridGraph->RemoveConnectionsToAdjacentNodes(66);
	m_pGridGraph->RemoveConnectionsToAdjacentNodes(67);
	m_pGridGraph->RemoveConnectionsToAdjacentNodes(47);
}

void App_ResearchTopic::UpdateImGui()
{
#ifdef PLATFORM_WINDOWS
#pragma region UI
	//UI
	{
		//Setup
		int menuWidth = 200;
		int const width = DEBUGRENDERER2D->GetActiveCamera()->GetWidth();
		int const height = DEBUGRENDERER2D->GetActiveCamera()->GetHeight();
		bool windowActive = true;
		ImGui::SetNextWindowPos(ImVec2((float)width - menuWidth - 10, 10));
		ImGui::SetNextWindowSize(ImVec2((float)menuWidth, (float)height - 20));
		ImGui::Begin("Gameplay Programming", &windowActive, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);
		ImGui::PushAllowKeyboardFocus(false);

		//Elements
		ImGui::Text("CONTROLS");
		ImGui::Indent();
		ImGui::Text("MMB: target");
		ImGui::Unindent();

		/*Spacing*/ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

		ImGui::Text("STATS");
		ImGui::Indent();
		ImGui::Text("%.3f ms/frame", 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("%.1f FPS", ImGui::GetIO().Framerate);
		ImGui::Unindent();

		/*Spacing*/ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing(); ImGui::Spacing();

		ImGui::Text("Vector Field Research topic");
		ImGui::Spacing();

		std::string buttonText{ "" };

		buttonText += "Destination Node";

		ImGui::Checkbox("HeatMap", &m_DebugSettings.showHeatMap);
		ImGui::Checkbox("NumberHeatMap", &m_DebugSettings.showNumberHeatMap);
		ImGui::Checkbox("VectorMap", &m_DebugSettings.showVectorMap);
		ImGui::Spacing();

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#pragma endregion
#endif
}

void App_ResearchTopic::CalculateHeatMap()
{
	m_HeatMap.clear();
	m_HeatMap.resize(COLUMNS * ROWS);
	std::fill(m_HeatMap.begin(), m_HeatMap.end(), -1);

	std::vector<Elite::GridTerrainNode*> toDoList{};
	std::vector< Elite::GridTerrainNode*> finishedList{};

	toDoList.push_back(m_pGridGraph->GetNode(destinationIdx));

	int distnaceCounter{ 0 };

	//For as long as there are nodes that have not been calculated
	while (toDoList.empty() == false)
	{
		std::vector<Elite::GridTerrainNode*> newToDoList{};

		//for every node in to do list
		for (Elite::GridTerrainNode* node : toDoList)
		{
			//Set distance
			m_HeatMap[node->GetIndex()] = distnaceCounter;

			//Check their connections to set up future caluclation
			for (const Elite::GraphConnection* connection :m_pGridGraph->GetConnections(*node))
			{
				bool IsAddedAlready{ false };

				// Setup node to check
				int nodeToCheck{};
				if (connection->GetTo() != node->GetIndex())
					nodeToCheck = connection->GetTo();
				else
					nodeToCheck = connection->GetFrom();

				//Check if it is in toDoList
				for (const Elite::GridTerrainNode* todoNode : toDoList)
				{
					if (todoNode->GetIndex() == nodeToCheck)
					{
						IsAddedAlready = true;
						break;
					}
				}
				//Check if it is in newToDoList
				for (const Elite::GridTerrainNode* newTodoNode : newToDoList)
				{
					if (newTodoNode->GetIndex() == nodeToCheck || IsAddedAlready)
					{
						IsAddedAlready = true;
						break;
					}
				}
				//Check if it is in finishedList
				for (const Elite::GridTerrainNode* finishedNode : finishedList)
				{
					if (finishedNode->GetIndex() == nodeToCheck || IsAddedAlready)
					{
						IsAddedAlready = true;
						break;
					}
				}
				//if it is in neither, add it to newToDoList
				if (IsAddedAlready == false)
				{
					newToDoList.push_back(m_pGridGraph->GetNode(nodeToCheck));
				}
			}
			finishedList.push_back(node);
		}

		toDoList = newToDoList;
		++distnaceCounter;
	}


	

}

void App_ResearchTopic::UpdateVectorMap()
{
	m_VectorMap.clear();
	m_VectorMap.resize(m_HeatMap.size());
	std::fill(m_VectorMap.begin(), m_VectorMap.end(), Vector2{0,0});

}
