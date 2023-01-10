#ifndef CROWDPATHFINDING_APPLICATION_H
#define CROWDPATHFINDING_APPLICATION_H

#include "framework/EliteInterfaces/EIApp.h"

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
	};
	DebugSettings m_DebugSettings{};


	std::vector<Sector*> m_SectorPtrs;
	Elite::Vector2 m_Destination{};

	


	void HandleInput();
	void UpdateImGui();
	std::vector<uint8>* ParseMapDataForSectors(int idx);
	void MakeLevel();
	void MakePortals();
};


#endif
