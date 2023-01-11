//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_ResearchTopic.h"
#include "HeatmapManager.h"
#include "VectorMapManager.h"
#include "AgentManager.h"
#include "LevelManager.h"
extern "C" {
	#define STB_IMAGE_IMPLEMENTATION
	#include "stb_image.h"
}


using namespace Elite;

constexpr bool USE_MAP = true;

//Destructor
App_ResearchTopic::~App_ResearchTopic()
{
	SAFE_DELETE(m_pGridGraph);
	SAFE_DELETE(m_pGraphRenderer);
	SAFE_DELETE(m_pGraphEditor);

	SAFE_DELETE(m_pHeatMap);
	SAFE_DELETE(m_pVectorMap);

	SAFE_DELETE(m_pAgents);

	SAFE_DELETE(m_pLevel);
}

//Functions
void App_ResearchTopic::Start()
{
	m_pGraphEditor = new GraphEditor();
	m_pGraphRenderer = new GraphRenderer();
	//Set Camera
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(39.0f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(73.0f, 35.0f));

	//Create Graph and Level
	MakeGridGraphAndLevel();

	m_pHeatMap = new HeatmapManager(COLUMNS * ROWS, m_pGridGraph);
	m_pVectorMap = new VectorMapManager(COLUMNS * ROWS, m_pGridGraph);
	m_pAgents = new AgentManager(m_pGridGraph);

	//Setup default start path
	destinationIdx = 88;
	m_pLevel->CheckUpdatedWalls();
	m_pHeatMap->CalculateHeatMap(destinationIdx);
	m_pVectorMap->ResetVectorMap();
}

void App_ResearchTopic::Update(float deltaTime)
{
	UNREFERENCED_PARAMETER(deltaTime);

	HandleInput();

	//IMGUI
	UpdateImGui();

	//UPDATE/CHECK GRID HAS CHANGED
	if (m_pGraphEditor->UpdateGraph(m_pGridGraph))
	{
		m_pLevel->CheckUpdatedWalls();
		m_pHeatMap->CalculateHeatMap(destinationIdx);
		m_pVectorMap->ResetVectorMap();
	}

	//UPDATE AGENTS
	m_pAgents->UpdateAgentsAndCreateVectors(deltaTime, destinationIdx, m_SizeCell, m_pVectorMap, m_pHeatMap);

}

void App_ResearchTopic::Render(float deltaTime) const
{
	UNREFERENCED_PARAMETER(deltaTime);
	//Render grid
	//WAY TOO LOW FPS
	//m_pGraphRenderer->RenderGraph(m_pGridGraph, true, false, false, false, m_HeatMap);

	//Render end node on top if applicable
	if (destinationIdx != invalid_node_index)
	{
		m_pGraphRenderer->HighlightNodes(m_pGridGraph, { m_pGridGraph->GetNode(destinationIdx) }, END_NODE_COLOR);
	}

	if (m_DebugSettings.showHeatMap == true)
	{
		int furthestIdx{-1};
		for (int distance : *m_pHeatMap->GetHeatMap())
		{
			if (distance > furthestIdx && distance != INT_MAX)
				furthestIdx = distance;
		}
		for (int index{}; index < m_pHeatMap->GetHeatMap()->size(); ++index)
		{
			if ((*m_pHeatMap->GetHeatMap())[index] != -1)
				m_pGraphRenderer->HighlightNodes(m_pGridGraph, { m_pGridGraph->GetNode(index) }, Elite::Color{ 1 - float((*m_pHeatMap->GetHeatMap())[index]) / furthestIdx, 0, 0 });
		}
	}

	if (m_DebugSettings.showVectorMap == true)
	{
		DrawVectors();
	}

	m_pAgents->RenderAgents(deltaTime);
}

void App_ResearchTopic::MakeGridGraphAndLevel()
{
	if (USE_MAP) 
	{
		COLUMNS = 100; ROWS = 100;
	}
	else
	{
		COLUMNS = 15; ROWS = 15;
	}
	m_pGridGraph = new GridGraph<GridTerrainNode, GraphConnection>(COLUMNS, ROWS, m_SizeCell, false, false, 1.f, 1.5f);

	m_pLevel = new LevelManager(COLUMNS * ROWS, m_SizeCell, m_pGridGraph);

	if (USE_MAP)
	{
		ParseMapData();
	}
	else
	{
		MakeSmallTestMap();
	}
}

void App_ResearchTopic::UpdateImGui()
{
#ifdef PLATFORM_WINDOWS
#pragma region UI
	//UI
	{
		//Setup
		int menuWidth = 250;
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
		ImGui::Text("MMB: Change goal");
		ImGui::Text("left mouse: Pan camera");
		ImGui::Text("Up arrow: Place 1 agent ");
		ImGui::Text("Right arrow: Place 30 agents");
		ImGui::Text("Down arrow: Clear all agents");
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
		ImGui::Text("Stats");
		ImGui::Indent();
		ImGui::Text("Amount of Cells: %.1f ", float(COLUMNS * ROWS));
		ImGui::Text("Amount of Agents: %.1f ", float(m_pAgents->GetAmountOfAgents()));
		ImGui::Unindent();
		ImGui::Spacing();

		std::string buttonText{ "" };

		buttonText += "Destination Node";

		ImGui::Checkbox("HeatMap", &m_DebugSettings.showHeatMap);
		ImGui::Checkbox("VectorMap", &m_DebugSettings.showVectorMap);
		ImGui::Spacing();

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#pragma endregion
#endif
}

void App_ResearchTopic::DrawVectors() const
{
	for (int idx{}; idx < m_pVectorMap->GetVectorMap()->size(); ++idx)
	{
		if (!((*m_pHeatMap->GetHeatMap())[idx] == 0 || (*m_pHeatMap->GetHeatMap())[idx] == -1))
		{
			Vector2 position{ m_pGridGraph->GetNodeWorldPos(idx) };
			DEBUGRENDERER2D->DrawSegment(position, position + (*m_pVectorMap->GetVectorMap())[idx] * 5, Color{ 1.f,1.f,1.f }, 0.9f);
		}
	}
}

void App_ResearchTopic::HandleInput()
{
	//INPUT
	bool const middleMousePressed = INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eMiddle);
	bool const upPressed = INPUTMANAGER->IsKeyboardKeyUp(InputScancode::eScancode_Up);
	bool const rightPressed = INPUTMANAGER->IsKeyboardKeyUp(InputScancode::eScancode_Right);
	bool const downPressed = INPUTMANAGER->IsKeyboardKeyUp(InputScancode::eScancode_Down);

	const MouseData mouseData = { INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eMiddle) };
	const Vector2 mousePos = { DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ (float)mouseData.X, (float)mouseData.Y }) };

	if (middleMousePressed)
	{
		//Find closest node to click pos
		const int closestNode = m_pGridGraph->GetNodeIdxAtWorldPos(mousePos);
		destinationIdx = closestNode;
		m_pHeatMap->CalculateHeatMap(destinationIdx);
		m_pVectorMap->ResetVectorMap();
	}
	if (upPressed)
	{
		m_pAgents->AddAgent(/*mousePos*/ DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld(Vector2{ 901/2.f, 451/2.f }));
	}
	if (rightPressed)
	{
		m_pAgents->Add30Agents(/*mousePos*/ DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld(Vector2{ 901 / 2.f, 451 / 2.f }));
	}
	if (downPressed)
	{
		m_pAgents->ClearAllAgents();
	}

}

void App_ResearchTopic::ParseMapData()
{
	//setup variables
	const std::string filename = "resources/map.png";
	int width, height;
	std::vector<unsigned char> image;

	//getting data out of png
	int n;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &n, 4);
	if (data != nullptr)
	{
		image = std::vector<unsigned char>(data, data + width * height * 4);
	}
	else
		std::cout << "Failed to load image\n";
	stbi_image_free(data);

	std::cout << "Image width = " << width << '\n';
	std::cout << "Image height = " << height << '\n';
	
	const size_t RGBA = 4;

	int amountOfWallsPlaced{};
	for (int i{}; i < COLUMNS * ROWS; ++i)
	{
		const int y = i / COLUMNS;
		const int x = i % COLUMNS;
		const size_t index = RGBA * ((height - 1 - y) * width + x);
		if (image[index + 0] == 0 && image[index + 1] == 0 && image[index + 2] == 0) // + 0 means R channel of pixel, + 1 means green...
		{
			m_pGridGraph->GetNode(i)->SetTerrainType(TerrainType::Water);
			m_pGridGraph->RemoveConnectionsToAdjacentNodes(i);
			m_pLevel->AddWall(i);
			++amountOfWallsPlaced;
			if (amountOfWallsPlaced % 100 == 0)
			{
				std::cout << "Placed " << amountOfWallsPlaced << " walls, more loading.\n";
			}
		}
	}
	std::cout << "Finished placing Walls!\nTotal of " << amountOfWallsPlaced << " walls placed.\n";
}

void Elite::App_ResearchTopic::MakeSmallTestMap()
{
	//Setup default terrain
	//HORIZONTAL
	for (int i{}; i < COLUMNS; ++i)
	{
		//BOTTOM
		m_pGridGraph->GetNode(i)->SetTerrainType(TerrainType::Water);
		m_pGridGraph->RemoveConnectionsToAdjacentNodes(i);
		m_pLevel->AddWall(i);
		//TOP
		m_pGridGraph->GetNode(COLUMNS * (ROWS - 1) + i)->SetTerrainType(TerrainType::Water);
		m_pGridGraph->RemoveConnectionsToAdjacentNodes(COLUMNS * (ROWS - 1) + i);
		m_pLevel->AddWall(COLUMNS * (ROWS - 1) + i);
	}
	//VERTICAL
	for (int i{}; i < ROWS; ++i)
	{
		//LEFT
		m_pGridGraph->GetNode(COLUMNS * i)->SetTerrainType(TerrainType::Water);
		m_pGridGraph->RemoveConnectionsToAdjacentNodes(COLUMNS * i);
		m_pLevel->AddWall(COLUMNS * i);
		//RIGHT
		m_pGridGraph->GetNode(COLUMNS * i + COLUMNS - 1)->SetTerrainType(TerrainType::Water);
		m_pGridGraph->RemoveConnectionsToAdjacentNodes(COLUMNS * i + COLUMNS - 1);
		m_pLevel->AddWall(COLUMNS * i + COLUMNS - 1);
	}
}
