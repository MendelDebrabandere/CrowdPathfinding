//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_CrowdPathfinding.h"
#include "Sector.h"
#include "AgentManager.h"
#include "projects/Movement/SteeringBehaviors/SteeringAgent.h"
#include "Portal.h"
#include "framework/EliteAI/EliteNavigation/ENavigation.h"

//#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAstar.h"
//#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EBFS.h"


extern "C" {
	#define STB_IMAGE_IMPLEMENTATION
	#include "stb_image.h"
}


using namespace Elite;

//Destructor
App_CrowdPathfinding::~App_CrowdPathfinding()
{
	for (Sector* pSector: m_SectorPtrs)
	{
		SAFE_DELETE(pSector);
	}
	SAFE_DELETE(m_pAgentManager);
}

//Functions
void App_CrowdPathfinding::Start()
{
	//Set Camera
	DEBUGRENDERER2D->GetActiveCamera()->SetZoom(39.0f);
	DEBUGRENDERER2D->GetActiveCamera()->SetCenter(Elite::Vector2(73.0f, 35.0f));

	//Create Graph and Level
	MakeLevel();

	//Setup default start path
	m_Destination = Vector2{ 45.5f,55.5f };

	for (Sector* pSector : m_SectorPtrs)
	{
		pSector->SetHeatFieldPoint(m_Destination, 0);
	}

	m_pAgentManager = new AgentManager();
}

void App_CrowdPathfinding::Update(float deltaTime)
{
	UNREFERENCED_PARAMETER(deltaTime);

	HandleInput();

	//IMGUI
	UpdateImGui();

	//UPDATE/CHECK GRID HAS CHANGED

	//UPDATE AGENTS
	DoPathCalculations();
}

void App_CrowdPathfinding::Render(float deltaTime) const
{
	UNREFERENCED_PARAMETER(deltaTime);
	//Render grid

	//Render end node on top if applicable
	DEBUGRENDERER2D->DrawPoint(m_Destination, 4, Color{ 0,1,0 });

	for (const Sector* sector: m_SectorPtrs)
	{
		sector->Draw(m_DebugSettings.showSectorBorders, m_DebugSettings.showSectorCells, m_DebugSettings.showPortals, m_DebugSettings.showHeatMap);
	}

	if (m_DebugSettings.showSectorsToCalc)
	{
		for (const Sector* sector : m_SectorPtrs)
		{
			bool isInCalc{ false };
			for (const Sector* sectorToCalc : m_SectorsToCalc)
			{
				if (sectorToCalc == sector)
				{
					isInCalc = true;
					break;
				}
			}
			if (isInCalc)
				sector->Draw(true, false, false, m_DebugSettings.showHeatMap);
			else
				sector->Draw(false, false, false, false);
		}
	}

	m_pAgentManager->RenderAgents(deltaTime);
}

void App_CrowdPathfinding::UpdateImGui()
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
		ImGui::Text("Stats");
		ImGui::Indent();
		ImGui::Text("Amount of Cells: %.1f ", 10000.f);
		//ImGui::Text("Amount of Agents: %.1f ", float(m_pAgents->GetAmountOfAgents()));
		ImGui::Unindent();
		ImGui::Spacing();

		std::string buttonText{ "" };

		buttonText += "Destination Node";

		ImGui::Checkbox("showSectorBorders", &m_DebugSettings.showSectorBorders);
		ImGui::Checkbox("showSectorCells", &m_DebugSettings.showSectorCells);
		ImGui::Checkbox("showPortals", &m_DebugSettings.showPortals);
		ImGui::Checkbox("showSectorsToCalc", &m_DebugSettings.showSectorsToCalc);
		ImGui::Checkbox("showHeatMap", &m_DebugSettings.showHeatMap);
		ImGui::Spacing();

		//End
		ImGui::PopAllowKeyboardFocus();
		ImGui::End();
	}
#pragma endregion
#endif
}

void App_CrowdPathfinding::MakeLevel()
{
	m_SectorPtrs.reserve(100);
	//100 by 100 level
	for (int y{}; y < s_Size; ++y)
	{
		for (int x{}; x < s_Size; ++x)
		{
			m_SectorPtrs.emplace_back(new Sector(Vector2{ float(5 + x * s_Size), float(5 + y * s_Size) }, ParseMapDataForSectors(x + y*s_Size), &m_SectorPtrs));
		}
	}

	MakePortals();
}

void App_CrowdPathfinding::HandleInput()
{
	//INPUT
	bool const middleMousePressed = INPUTMANAGER->IsMouseButtonUp(InputMouseButton::eMiddle);
	bool const upPressed = INPUTMANAGER->IsKeyboardKeyUp(InputScancode::eScancode_Up);
	bool const rightPressed = INPUTMANAGER->IsKeyboardKeyUp(InputScancode::eScancode_Right);
	bool const downPressed = INPUTMANAGER->IsKeyboardKeyUp(InputScancode::eScancode_Down);

	const MouseData mouseData = { INPUTMANAGER->GetMouseData(Elite::InputType::eMouseButton, Elite::InputMouseButton::eMiddle) };
	const Vector2 mousePos = { DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld({ (float)mouseData.X, (float)mouseData.Y }) };

	//if (middleMousePressed)
	//{
	//	//Find closest node to click pos
	//	const int closestNode = m_pGridGraph->GetNodeIdxAtWorldPos(mousePos);
	//	destinationIdx = closestNode;
	//	m_pHeatMap->CalculateHeatMap(destinationIdx);
	//	m_pVectorMap->ResetVectorMap();
	//}
	//if (upPressed)
	//{
	//	m_pAgents->AddAgent(/*mousePos*/ DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld(Vector2{ 901/2.f, 451/2.f }));
	//}
	//if (rightPressed)
	//{
	//	m_pAgents->Add30Agents(/*mousePos*/ DEBUGRENDERER2D->GetActiveCamera()->ConvertScreenToWorld(Vector2{ 901 / 2.f, 451 / 2.f }));
	//}
	//if (downPressed)
	//{
	//	m_pAgents->ClearAllAgents();
	//}

}

std::vector<uint8> App_CrowdPathfinding::ParseMapDataForSectors(int idx)
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

	constexpr size_t RGBA = 4;

	std::vector<uint8> vector{};

	vector.reserve(s_Size * s_Size);

	const int yStartPos{ (idx / 10) * 10 };
	for (int y{ yStartPos }; y < yStartPos + 10; ++y)
	{
		const int xStartPos{ (idx % 10) * 10 };
		for (int x{ xStartPos }; x < xStartPos + 10; ++x)
		{
			const size_t index = RGBA * ((height - 1 - y) * width + x);
			if ((image[index + 0] == 0 && image[index + 1] == 0) && image[index + 2] == 0) // if its black
			{
				vector.emplace_back(255);
			}
			else if ((image[index + 0] >= 1 && image[index + 1] >= 1) && image[index + 2] >= 1) // else if its white
			{
				vector.emplace_back(1);
			}
			else if ((image[index + 0] == 0 && image[index + 1] == 0) && image[index + 2] >= 1) // else if its blue
			{
				vector.emplace_back(254);
			}
			else if ((image[index + 0] >= 1 && image[index + 1] == 0) && image[index + 2] == 0) // else if its red
			{
				vector.emplace_back(2);
			}
			else std::cout << "Failed image parse, inpure color index is: " << index << '\n';
		}
	}
	return vector;
}

void App_CrowdPathfinding::MakePortals()
{
	for (Sector* pSector : m_SectorPtrs)
	{
		pSector->MakePortals(m_SectorPtrs);
	}
}

void App_CrowdPathfinding::DoPathCalculations()
{
	for (const SteeringAgent* agent : *m_pAgentManager->GetAgents())
	{
		const int agentSectorIdx{ (int(agent->GetPosition().x) / 10) + (10 * (int(agent->GetPosition().y) / 10)) };

		bool isChecking{ false };
		// check if it is already in the SectorsToCalc
		for (const Sector* pSector : m_SectorsToCalc)
		{
			const Vector2 center{ pSector->GetCenter() };
			const int sectorIdx{ (int(center.x) / 10) + (10 * (int(center.y) / 10)) };

			if (sectorIdx == agentSectorIdx)
			{
				isChecking = true;
				break;
			}
		}

		if (isChecking)
			continue;

		DoAstarAndAddToVec(m_SectorPtrs[agentSectorIdx]);
	}
	GenerateFlowFields();
}

void App_CrowdPathfinding::DoAstarAndAddToVec(Sector* pStartNode)
{
	std::vector<Sector*> path;
	std::vector<NodeRecord> openList;
	std::vector<NodeRecord> closedList;

	// 1. Create a NodeRecord to kickstart the loop
	NodeRecord currentRecord{ pStartNode, nullptr, 0.0f, GetHeuristicCost(pStartNode) };
	openList.push_back(currentRecord);

	// 2. While (openlist != empty)
	while (openList.size() != 0)
	{
		// A. Get NodeRecord with lowest costfrom openList
		currentRecord = *std::min_element(openList.begin(), openList.end());

		// B. Check if that connection leads to the end node
		const Sector* pGoalNode{ m_SectorPtrs[(int(m_Destination.x) / 10) + (10 * (int(m_Destination.y) / 10))] };
		if (currentRecord.pNode == pGoalNode)
			break;

		// C. Else we get all the connections of the NodeRecor's node
		for (const auto& connection : *currentRecord.pNode->GetPortals())
		{
			// D. Check if any of those connections lead to a node already in the closed list
			Sector* pConnectedNode{ m_SectorPtrs[connection->GetTo()] };

			const float currentCost{ currentRecord.costSoFar + 1 };

			bool foundCheaper{ false };

			for (const NodeRecord& NRecord : closedList)
			{
				if (NRecord.pNode == pConnectedNode)
				{
					if (NRecord.costSoFar < currentCost)
					{
						foundCheaper = true;
						break;
					}

					closedList.erase(std::remove(closedList.begin(), closedList.end(), NRecord));
					break;
				}
			}
			if (foundCheaper) continue;

			// E. If 2.D check failed, check if any of those connections lead to a node already in the open list
			for (const NodeRecord& upcomingRecord : openList)
			{
				if (upcomingRecord.pNode == pConnectedNode)
				{
					if (upcomingRecord.costSoFar < currentCost)
					{
						foundCheaper = true;
						break;
					}
					openList.erase(std::remove(openList.begin(), openList.end(), upcomingRecord));
					break;
				}
			}
			if (foundCheaper) continue;


			// F.	At this point any expensive connection should be removed (if it existed).
			//		We create a new nodeRecord and add it to the openList
			openList.push_back(NodeRecord{ pConnectedNode, connection, currentCost, currentCost + GetHeuristicCost(pConnectedNode) });
		}

		// G. Remove NodeRecord from the openList and add it to the closedList
		openList.erase(std::remove(openList.begin(), openList.end(), currentRecord));
		closedList.push_back(currentRecord);
	}

	// 3. Reconstruct path from last connection to start node
	while (currentRecord.pNode != pStartNode)
	{
		path.push_back(currentRecord.pNode);

		for (const NodeRecord& Node : closedList)
		{
			//std::cout << int(int(Node.pNode->GetCenter().x) / 10) + (10 * (int(Node.pNode->GetCenter().y)) / 10) << '\n';
			if (currentRecord.pConnection->GetFrom() == (int(Node.pNode->GetCenter().x) / 10) + (10 * (int(Node.pNode->GetCenter().y) / 10)))
			{
				currentRecord = Node;
				break;
			}
		}
	}


	path.push_back(pStartNode);


	//std::reverse(path.begin(), path.end());
	
	//return path;
	for (Sector* pathSector : path)
	{
		bool isInSectorToCalc{ false };
		for (const Sector* sector : m_SectorsToCalc)
		{
			if (sector == pathSector)
			{
				isInSectorToCalc = true;
				break;
			}
		}
		if (isInSectorToCalc == false)
		{
			m_SectorsToCalc.push_back(pathSector);
		}

	}
}

float App_CrowdPathfinding::GetHeuristicCost(const Sector* pStartNode) const
{
	const Vector2 toDestination = m_Destination - pStartNode->GetCenter();
	return HeuristicFunctions::Chebyshev(abs(toDestination.x), abs(toDestination.y));
	
}

void App_CrowdPathfinding::GenerateFlowFields()
{
	for (Sector* sector : m_SectorsToCalc)
	{
		sector->GenerateFlowField();
	}
}
