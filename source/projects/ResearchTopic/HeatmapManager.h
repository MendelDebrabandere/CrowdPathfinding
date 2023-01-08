#pragma once
#include "framework/EliteAI/EliteGraphs/EGridGraph.h"

namespace Elite
{
	class HeatmapManager final
	{
	public:
		//Constructor & Destructor
		HeatmapManager(int size, GridGraph<GridTerrainNode, GraphConnection>* gridPtr);
		//~HeatmapManager();

		// rule of 5 copypasta
		//HeatmapManager(const HeatmapManager& other) = delete;
		//HeatmapManager(HeatmapManager&& other) = delete;
		//HeatmapManager& operator=(const HeatmapManager& other) = delete;
		//HeatmapManager& operator=(HeatmapManager&& other) = delete;

		const std::vector<int>* GetHeatMap() const;

		void CalculateHeatMap(int destinationIdx);


	private:
		std::vector<int> m_HeatMap{};
		GridGraph<GridTerrainNode, GraphConnection>* m_pGridGraph{};

	};
}
