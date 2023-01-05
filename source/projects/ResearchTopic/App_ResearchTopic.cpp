//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_ResearchTopic.h"
#include "projects/Movement/SteeringBehaviors/SteeringAgent.h"


using namespace Elite;

//Destructor
App_ResearchTopic::~App_ResearchTopic()
{
	SAFE_DELETE(m_pGridGraph);
	SAFE_DELETE(m_pGraphRenderer);
	SAFE_DELETE(m_pGraphEditor);

	for (auto a : m_AgentVec)
	{
		SAFE_DELETE(a.pAgent);
	}
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
	if (INPUTMANAGER->IsKeyboardKeyUp(InputScancode::eScancode_Up))
	{
		MouseData data{ INPUTMANAGER->GetMouseData(InputType{}) };
		AddAgent(Vector2{ float(data.X), float(data.Y) });
	}

	//IMGUI
	UpdateImGui();

	//UPDATE/CHECK GRID HAS CHANGED
	if (m_pGraphEditor->UpdateGraph(m_pGridGraph))
	{
		CalculateHeatMap();
	}

	//UPDATE AGENTS
	for (const ImGui_Agent& a : m_AgentVec)
	{
		if (a.pAgent)
		{
			a.pAgent->SetLinearVelocity(m_VectorMap[m_pGridGraph->GetNodeIdxAtWorldPos(a.pAgent->GetPosition())] * 15);
			a.pAgent->Update(deltaTime);
		}
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
			if (m_HeatMap[index] != -1)
				m_pGraphRenderer->HighlightNodes(m_pGridGraph, { m_pGridGraph->GetNode(index) }, Elite::Color{ 1 - float(m_HeatMap[index]) / furthestIdx, 0, 0 });
			else
				m_pGraphRenderer->HighlightNodes(m_pGridGraph, { m_pGridGraph->GetNode(index) }, Elite::Color{ 0,0,1});
		}
	}

	if (m_DebugSettings.showVectorMap == true)
	{
		DrawVectors();
	}

	for (const ImGui_Agent& a : m_AgentVec)
	{
		if (a.pAgent)
		{
			a.pAgent->Render(deltaTime);
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
	UpdateVectorMap();
}

void App_ResearchTopic::UpdateVectorMap()
{
	m_VectorMap.clear();
	m_VectorMap.resize(m_HeatMap.size());
	std::fill(m_VectorMap.begin(), m_VectorMap.end(), Vector2{0,0});

	// for all nodes on the grid
	for (int index{}; index < m_VectorMap.size(); ++index)
	{
		m_VectorMap[index] = CreateVector(index);
	}
}

Vector2 App_ResearchTopic::CreateVector(int idx) const
{
	if (m_HeatMap[idx] == -1)
	{
		return Vector2{};
	}

	std::vector<Vector2> surroundingVectors{};
	surroundingVectors.reserve(8);

	const float sqrt2{ 1 / sqrtf(2) };
	Vector2 centerPos{ m_pGridGraph->GetNodeWorldPos(idx) };

	bool isNextToWall{ false };

	{	//Left node
		int nodeIdx{ m_pGridGraph->GetNodeIdxAtWorldPos(centerPos + Vector2{-float(m_SizeCell), 0}) };
		if (nodeIdx != invalid_node_index && m_HeatMap[nodeIdx] != -1)
		{
			surroundingVectors.push_back(Vector2{ -float(m_HeatMap[nodeIdx]), 0 });
		}
		else
			isNextToWall = true;
		
	}
	{	//TopLeft node
		int nodeIdx{ m_pGridGraph->GetNodeIdxAtWorldPos(centerPos + Vector2{-float(m_SizeCell), float(m_SizeCell)}) };
		if (nodeIdx != invalid_node_index && m_HeatMap[nodeIdx] != -1)
		{
			surroundingVectors.push_back(Vector2{sqrt2 *  -float(m_HeatMap[nodeIdx]), sqrt2 * float(m_HeatMap[nodeIdx]) });
		}
		else
			isNextToWall = true;
	}
	{	//Top node
		int nodeIdx{ m_pGridGraph->GetNodeIdxAtWorldPos(centerPos + Vector2{0, float(m_SizeCell)}) };
		if (nodeIdx != invalid_node_index && m_HeatMap[nodeIdx] != -1)
		{
			surroundingVectors.push_back(Vector2{ 0, float(m_HeatMap[nodeIdx]) });
		}
		else
			isNextToWall = true;
	}
	{	//TopRight node
		int nodeIdx{ m_pGridGraph->GetNodeIdxAtWorldPos(centerPos + Vector2{float(m_SizeCell), float(m_SizeCell)}) };
		if (nodeIdx != invalid_node_index && m_HeatMap[nodeIdx] != -1)
		{
			surroundingVectors.push_back(Vector2{ sqrt2 * float(m_HeatMap[nodeIdx]),sqrt2 * float(m_HeatMap[nodeIdx]) });
		}
		else
			isNextToWall = true;
	}
	{	//Right node
		int nodeIdx{ m_pGridGraph->GetNodeIdxAtWorldPos(centerPos + Vector2{float(m_SizeCell), 0}) };
		if (nodeIdx != invalid_node_index && m_HeatMap[nodeIdx] != -1)
		{
			surroundingVectors.push_back(Vector2{ float(m_HeatMap[nodeIdx]), 0 });
		}
		else
			isNextToWall = true;
	}
	{	//BottomRight node
		int nodeIdx{ m_pGridGraph->GetNodeIdxAtWorldPos(centerPos + Vector2{float(m_SizeCell), -float(m_SizeCell)}) };
		if (nodeIdx != invalid_node_index && m_HeatMap[nodeIdx] != -1)
		{
			surroundingVectors.push_back(Vector2{ sqrt2 * float(m_HeatMap[nodeIdx]),sqrt2 * -float(m_HeatMap[nodeIdx]) });
		}
		else
			isNextToWall = true;
	}
	{	//Bottom node
		int nodeIdx{ m_pGridGraph->GetNodeIdxAtWorldPos(centerPos + Vector2{0, -float(m_SizeCell)}) };
		if (nodeIdx != invalid_node_index && m_HeatMap[nodeIdx] != -1)
		{
			surroundingVectors.push_back(Vector2{ 0, -float(m_HeatMap[nodeIdx]) });
		}
		else
			isNextToWall = true;
	}
	{	//BottomLeft node
		int nodeIdx{ m_pGridGraph->GetNodeIdxAtWorldPos(centerPos + Vector2{-float(m_SizeCell), -float(m_SizeCell)}) };
		if (nodeIdx != invalid_node_index && m_HeatMap[nodeIdx] != -1)
		{
			surroundingVectors.push_back(Vector2{ sqrt2 * -float(m_HeatMap[nodeIdx]),sqrt2 * -float(m_HeatMap[nodeIdx]) });
		}
		else
			isNextToWall = true;
	}


	Vector2 returnVector{};
	for (const Vector2& vector: surroundingVectors)
	{
		//If the square is next to a wall
		if (isNextToWall)
		{
			// find the smallest vector
			Vector2 smallestVector{1000, 1000};
			for (const Vector2& vector : surroundingVectors)
			{
				if (smallestVector.MagnitudeSquared() > vector.MagnitudeSquared())
					smallestVector = vector;
			}
			returnVector = -smallestVector;
			break;
		}

		returnVector += 1/vector;
	}

	returnVector.Normalize();

	return -returnVector;
}

void App_ResearchTopic::DrawVectors() const
{
	for (int idx{}; idx < m_VectorMap.size(); ++idx)
	{
		if (m_HeatMap[idx] != 0)
		{
			Vector2 position{ m_pGridGraph->GetNodeWorldPos(idx) };
			DEBUGRENDERER2D->DrawSegment(position, position + m_VectorMap[idx] * 5, Color{ 1.f,1.f,1.f }, 0.9f);
		}
	}
}

void App_ResearchTopic::AddAgent(const Vector2& position)
{
	ImGui_Agent agent{};
	agent.pAgent = new SteeringAgent();
	agent.pAgent->SetAutoOrient(true);
	agent.pAgent->SetMaxLinearSpeed(15);
	agent.pAgent->SetMass(1);
	agent.pAgent->SetPosition(Vector2{100,50});

	m_AgentVec.push_back(agent);
}