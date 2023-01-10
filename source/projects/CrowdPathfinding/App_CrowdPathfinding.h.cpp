//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"

//Includes
#include "App_CrowdPathfinding.h"
#include "Sector.h"

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
	m_Destination = Vector2{ 50,50 };
}

void App_CrowdPathfinding::Update(float deltaTime)
{
	UNREFERENCED_PARAMETER(deltaTime);

	HandleInput();

	//IMGUI
	UpdateImGui();

	//UPDATE/CHECK GRID HAS CHANGED

	//UPDATE AGENTS

}

void App_CrowdPathfinding::Render(float deltaTime) const
{
	UNREFERENCED_PARAMETER(deltaTime);
	//Render grid

	//Render end node on top if applicable
	DEBUGRENDERER2D->DrawPoint(m_Destination, 4, Color{ 0,1,0 });

	for (const Sector* sector: m_SectorPtrs)
	{
		sector->Draw(m_DebugSettings.showSectorBorders, m_DebugSettings.showSectorCells, m_DebugSettings.showPortals);
	}

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
			m_SectorPtrs.emplace_back(new Sector(Vector2{ float(5 + x * s_Size), float(5 + y * s_Size) }, ParseMapDataForSectors(x + y*s_Size)));
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

std::vector<uint8>* App_CrowdPathfinding::ParseMapDataForSectors(int idx)
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

	std::vector<uint8>* pVector = new std::vector<uint8>{};

	pVector->reserve(s_Size * s_Size);

	const int yStartPos{ (idx / 10) * 10 };
	for (int y{ yStartPos }; y < yStartPos + 10; ++y)
	{
		const int xStartPos{ (idx % 10) * 10 };
		for (int x{ xStartPos }; x < xStartPos + 10; ++x)
		{
			const size_t index = RGBA * ((height - 1 - y) * width + x);
			if (image[index + 1] == 0) // + 0 means R channel of pixel, + 1 means green...
			{
				pVector->emplace_back(255);
			}
			else
			{
				pVector->emplace_back(1);
			}
		}
	}
	return pVector;
}

void App_CrowdPathfinding::MakePortals()
{
	for (Sector* pSector : m_SectorPtrs)
	{
		pSector->MakePortals(m_SectorPtrs);
	}
}
