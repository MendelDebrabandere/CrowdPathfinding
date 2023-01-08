#include "stdafx.h"
#include "HeatmapManager.h"

using namespace Elite;

HeatmapManager::HeatmapManager(int size, GridGraph<GridTerrainNode, GraphConnection>* gridPtr)
	:m_pGridGraph{gridPtr}
{
	m_HeatMap.resize(size);
}

const std::vector<int>* HeatmapManager::GetHeatMap() const
{
	return &m_HeatMap;
}

void HeatmapManager::CalculateHeatMap(int destinationIdx)
{
	const UINT size{ m_HeatMap.size() };
	m_HeatMap.clear();
	m_HeatMap.resize(size);
	std::fill(m_HeatMap.begin(), m_HeatMap.end(), -1);

	std::vector<Elite::GridTerrainNode*> toDoList{};
	std::vector< Elite::GridTerrainNode*> finishedList{};

	toDoList.push_back(m_pGridGraph->GetNode(destinationIdx));

	int distanceCounter{ 0 };

	//For as long as there are nodes that have not been calculated
	while (toDoList.empty() == false)
	{
		std::vector<GridTerrainNode*> newToDoList{};

		//for every node in to do list
		for (GridTerrainNode* node : toDoList)
		{
			//Set distance
			m_HeatMap[node->GetIndex()] = distanceCounter;

			//Check their connections to set up future caluclation
			for (const GraphConnection* connection : m_pGridGraph->GetConnections(*node))
			{
				bool IsAddedAlready{ false };

				// Setup node to check
				int nodeToCheck{};
				if (connection->GetTo() != node->GetIndex())
					nodeToCheck = connection->GetTo();
				else
					nodeToCheck = connection->GetFrom();

				//Check if it is in toDoList
				for (const GridTerrainNode* todoNode : toDoList)
				{
					if (todoNode->GetIndex() == nodeToCheck)
					{
						IsAddedAlready = true;
						break;
					}
				}
				//Check if it is in newToDoList
				for (const GridTerrainNode* newTodoNode : newToDoList)
				{
					if (newTodoNode->GetIndex() == nodeToCheck || IsAddedAlready)
					{
						IsAddedAlready = true;
						break;
					}
				}
				//Check if it is in finishedList
				for (const GridTerrainNode* finishedNode : finishedList)
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
		++distanceCounter;
	}

}
