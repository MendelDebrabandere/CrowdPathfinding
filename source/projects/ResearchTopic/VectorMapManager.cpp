#include "stdafx.h"
#include "VectorMapManager.h"
#include "HeatmapManager.h"

using namespace Elite;

VectorMapManager::VectorMapManager(int size, GridGraph<GridTerrainNode, GraphConnection>* gridPtr)
	:m_pGridGraph{ gridPtr }
{
	m_VectorMap.resize(size);
	std::fill(m_VectorMap.begin(), m_VectorMap.end(), Vector2{ 0,0 });
}

const std::vector<Vector2>* VectorMapManager::GetVectorMap() const
{
	return &m_VectorMap;
}

void VectorMapManager::ResetVectorMap()
{
	const UINT size{ m_VectorMap.size() };
	m_VectorMap.clear();
	m_VectorMap.resize(size);
}

void VectorMapManager::CreateAndSetVector(int idx, int destinationIdx, int sizeCell, const HeatmapManager* pHeatmapManager)
{
	const std::vector<int>* pHeatmap{ pHeatmapManager->GetHeatMap() };


	if ((*pHeatmap)[idx] == -1)
	{
		m_VectorMap[idx] = Vector2{};
		return;
	}

	std::vector<Vector2> surroundingVectors{};
	surroundingVectors.reserve(8);

	const float sqrt2{ 1 / sqrtf(2) };
	Vector2 centerPos{ m_pGridGraph->GetNodeWorldPos(idx) };

	bool isNextToWall{ false };

	{	//Left node
		int nodeIdx{ m_pGridGraph->GetNodeIdxAtWorldPos(centerPos + Vector2{-float(sizeCell), 0}) };
		if (nodeIdx != invalid_node_index && (*pHeatmap)[nodeIdx] != -1)
		{
			surroundingVectors.push_back(Vector2{ -float((*pHeatmap)[nodeIdx]), 0 });
		}
		else
			isNextToWall = true;

	}
	{	//TopLeft node
		int nodeIdx{ m_pGridGraph->GetNodeIdxAtWorldPos(centerPos + Vector2{-float(sizeCell), float(sizeCell)}) };
		if (nodeIdx != invalid_node_index && (*pHeatmap)[nodeIdx] != -1)
		{
			surroundingVectors.push_back(Vector2{ sqrt2 * -float((*pHeatmap)[nodeIdx]), sqrt2 * float((*pHeatmap)[nodeIdx]) });
		}
		else
			isNextToWall = true;
	}
	{	//Top node
		int nodeIdx{ m_pGridGraph->GetNodeIdxAtWorldPos(centerPos + Vector2{0, float(sizeCell)}) };
		if (nodeIdx != invalid_node_index && (*pHeatmap)[nodeIdx] != -1)
		{
			surroundingVectors.push_back(Vector2{ 0, float((*pHeatmap)[nodeIdx]) });
		}
		else
			isNextToWall = true;
	}
	{	//TopRight node
		int nodeIdx{ m_pGridGraph->GetNodeIdxAtWorldPos(centerPos + Vector2{float(sizeCell), float(sizeCell)}) };
		if (nodeIdx != invalid_node_index && (*pHeatmap)[nodeIdx] != -1)
		{
			surroundingVectors.push_back(Vector2{ sqrt2 * float((*pHeatmap)[nodeIdx]),sqrt2 * float((*pHeatmap)[nodeIdx]) });
		}
		else
			isNextToWall = true;
	}
	{	//Right node
		int nodeIdx{ m_pGridGraph->GetNodeIdxAtWorldPos(centerPos + Vector2{float(sizeCell), 0}) };
		if (nodeIdx != invalid_node_index && (*pHeatmap)[nodeIdx] != -1)
		{
			surroundingVectors.push_back(Vector2{ float((*pHeatmap)[nodeIdx]), 0 });
		}
		else
			isNextToWall = true;
	}
	{	//BottomRight node
		int nodeIdx{ m_pGridGraph->GetNodeIdxAtWorldPos(centerPos + Vector2{float(sizeCell), -float(sizeCell)}) };
		if (nodeIdx != invalid_node_index && (*pHeatmap)[nodeIdx] != -1)
		{
			surroundingVectors.push_back(Vector2{ sqrt2 * float((*pHeatmap)[nodeIdx]),sqrt2 * -float((*pHeatmap)[nodeIdx]) });
		}
		else
			isNextToWall = true;
	}
	{	//Bottom node
		int nodeIdx{ m_pGridGraph->GetNodeIdxAtWorldPos(centerPos + Vector2{0, -float(sizeCell)}) };
		if (nodeIdx != invalid_node_index && (*pHeatmap)[nodeIdx] != -1)
		{
			surroundingVectors.push_back(Vector2{ 0, -float((*pHeatmap)[nodeIdx]) });
		}
		else
			isNextToWall = true;
	}
	{	//BottomLeft node
		int nodeIdx{ m_pGridGraph->GetNodeIdxAtWorldPos(centerPos + Vector2{-float(sizeCell), -float(sizeCell)}) };
		if (nodeIdx != invalid_node_index && (*pHeatmap)[nodeIdx] != -1)
		{
			surroundingVectors.push_back(Vector2{ sqrt2 * -float((*pHeatmap)[nodeIdx]),sqrt2 * -float((*pHeatmap)[nodeIdx]) });
		}
		else
			isNextToWall = true;
	}


	Vector2 returnVector{};
	for (const Vector2& vector : surroundingVectors)
	{
		//If the square is next to a wall
		if (isNextToWall)
		{
			// find the smallest vector
			Vector2 smallestVector{ 10000, 0 };
			for (const Vector2& vector : surroundingVectors)
			{
				// if the vector is smaller than the cached vector + It has to can't be a diagonal vector
				if (smallestVector.MagnitudeSquared() > vector.MagnitudeSquared() && (vector.x == 0.f || vector.y == 0.f))
					smallestVector = vector;
			}
			if (smallestVector.MagnitudeSquared() <= 0.1f)
			{
				smallestVector = Vector2{ m_pGridGraph->GetNodeWorldPos(destinationIdx) - centerPos };
			}
			returnVector = -smallestVector;
			break;
		}

		returnVector += 1 / vector;
	}

	returnVector.Normalize();

	m_VectorMap[idx] = - returnVector;

}
