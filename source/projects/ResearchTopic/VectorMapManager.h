#pragma once
#include "framework/EliteAI/EliteGraphs/EGridGraph.h"

namespace Elite
{
	class HeatmapManager;

	class VectorMapManager final
	{
	public:
		//Constructor & Destructor
		VectorMapManager(int size, GridGraph<GridTerrainNode, GraphConnection>* gridPtr);
		//~HeatmapManager();

		// rule of 5 copypasta
		//VectorMapManager(const VectorMapManager& other) = delete;
		//VectorMapManager(VectorMapManager&& other) = delete;
		//VectorMapManager& operator=(const VectorMapManager& other) = delete;
		//VectorMapManager& operator=(VectorMapManager&& other) = delete;

		const std::vector<Vector2>* GetVectorMap() const;

		void ResetVectorMap();
		void CreateAndSetVector(int idx, int destinationIdx, int sizeCell, const HeatmapManager* pHeatmapManager);


	private:
		std::vector<Vector2> m_VectorMap{};
		GridGraph<GridTerrainNode, GraphConnection>* m_pGridGraph{};
	};
}
