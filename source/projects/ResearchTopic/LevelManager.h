#pragma once
#include "framework/EliteAI/EliteGraphs/EGridGraph.h"


namespace Elite
{
	class LevelManager final
	{
	public:
		//Constructor & Destructor
		LevelManager(int size, int sizeCell, GridGraph<GridTerrainNode, GraphConnection>* gridPtr);
		~LevelManager();

		//rule of 5 copypasta
		LevelManager(const LevelManager& other) = delete;
		LevelManager(LevelManager&& other) = delete;
		LevelManager& operator=(const LevelManager& other) = delete;
		LevelManager& operator=(LevelManager&& other) = delete;

		void AddWall(int idx);
		void CheckUpdatedWalls();

	private:
		int m_SizeCell{};
		std::vector<RigidBody*> m_WallRBPtrs{};
		GridGraph<GridTerrainNode, GraphConnection>* m_pGridGraph{};

	};

}

