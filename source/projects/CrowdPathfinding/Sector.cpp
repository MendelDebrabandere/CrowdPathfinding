#include "stdafx.h"
#include "Sector.h"
#include "Portal.h"

using namespace Elite;


Sector::Sector(const Elite::Vector2& center, const std::vector<uint8>& costField, std::vector<Sector*>* sectors)
	:m_Center{ center }
	, m_CostField{ costField }
	, m_pSectors{ sectors }
{

	//Create Rigidbodies
	m_RBptrs.resize(costField.size());
	const RigidBodyDefine define = RigidBodyDefine(0.01f, 0.1f, eStatic, false);
	for (int i{}; i < costField.size(); ++i)
	{
		if (costField[i] == 255)
		{																			
			const Transform transform = Transform(m_Center + Vector2{ -(s_Cells/2.f - s_CellSize/2.f) + i % s_Cells,
																	  -(s_Cells/2.f - s_CellSize/2.f) + i / s_Cells }, ZeroVector2);
			m_RBptrs[i] = new RigidBody(define, transform);
			//Add shape
			EPhysicsBoxShape shape;
			shape.width = s_CellSize;
			shape.height = s_CellSize;
			m_RBptrs[i]->AddShape(&shape);
		}
	}

	// set vector sizes
	m_HeatField.resize(m_CostField.size());
	std::fill(m_HeatField.begin(), m_HeatField.end(), -1);
	m_FlowField.resize(m_CostField.size());
}

Sector::~Sector()
{

	for (int i{}; i < m_RBptrs.size(); ++i)
	{
		SAFE_DELETE(m_RBptrs[i]);
	}

	for (int i{}; i < m_PortalsPtrs.size(); ++i)
	{
		SAFE_DELETE(m_PortalsPtrs[i]);
	}

}

void Sector::Draw(bool drawEdges, bool drawCells, bool showPortals, bool showHeatMap) const
{
	constexpr float halfCells{ s_Cells / 2.f };
	if (drawEdges)
	{
		const std::vector<Vector2> polygon{ m_Center + Vector2{-halfCells, -halfCells},
									m_Center + Vector2{-halfCells, halfCells},
									m_Center + Vector2{halfCells, halfCells},
									m_Center + Vector2{halfCells, -halfCells} };
		DEBUGRENDERER2D->DrawPolygon(polygon.data(), 4, Color{ 0.0f,0.0f,0.6f }, 0.7f);
	}
	if (drawCells)
	{
		for (uint8 idx{}; idx < m_CostField.size(); ++idx)
		{
			const std::vector<Vector2> polygon{ m_Center + Vector2{float(-halfCells + idx % s_Cells), float(-halfCells + idx / s_Cells)},
												m_Center + Vector2{float(-halfCells + idx % s_Cells), float(-halfCells + idx / s_Cells + s_CellSize)},
												m_Center + Vector2{float(-halfCells + idx % s_Cells + s_CellSize), float(-halfCells + idx / s_Cells + s_CellSize)},
												m_Center + Vector2{float(-halfCells + idx % s_Cells + s_CellSize), float(-halfCells + idx / s_Cells)} };

			if (m_CostField[idx] == 255)
			{
				DEBUGRENDERER2D->DrawPolygon(polygon.data(), 4, Color{ 1,0,0 }, 0.9f);
			}
			else
			{
				DEBUGRENDERER2D->DrawPolygon(polygon.data(), 4, Color{ 0.7f,0.7f,0.7f }, 0.8f);
			}
		}
	}
	if (showPortals)
	{
		for(const Portal* portal : m_PortalsPtrs)
		{
			portal->Draw();
		}
	}
	if (showHeatMap)
	{
		if (m_HasFlowFieldGenerated)
		{
			float highestHeat{ -2 };
			for (float cell : m_HeatField)
			{
				if (cell > highestHeat)
					highestHeat = cell;
			}


			for (int idx{}; idx < m_HeatField.size(); ++idx)
			{
				const Vector2 center{ m_Center + Vector2{	-4.5f + float(-halfCells + idx % s_Cells) + s_Cells / 2.f,
															-4.5f + float(-halfCells + idx / s_Cells) + s_Cells / 2.f} };

				if (m_HeatField[idx] != -1)
					DEBUGRENDERER2D->DrawPoint(center, 4, Color{ 1 - m_HeatField[idx] / highestHeat, 0, 0 });
				else
					DEBUGRENDERER2D->DrawPoint(center, 4, Color{ 0, 0.5f, 1 });
			}
		}
	}
}

void Sector::MakePortals(const std::vector<Sector*>& sectorPtrs)
{
	//idx of 'this' in the vector
	int myIdx{};
	for (int idx{}; idx < sectorPtrs.size(); ++idx)
	{
		if (m_Center == sectorPtrs[idx]->GetCenter())
		{
			myIdx = idx;
			break;
		}
	}

	{	// TOP SIDE

		//Check if this sector has a sector above it
		if (myIdx < 90)
		{
			bool isMakingPortal{ false };
			Vector2 startPortalPos{};
			//for the top row
			for (size_t idx{ m_CostField.size() - s_Cells }; idx < m_CostField.size(); ++idx)
			{
				//Check if there is a wall at the idx
				if (m_CostField[idx] <= 200)
				{
					//Check if there is a wall on the other side
					if (sectorPtrs[myIdx + 10]->IsWall(idx - 90) == false)
					{
						//MAKE PORTAL
						//Start of portal?
						if (isMakingPortal == false)
						{
							isMakingPortal = true;
							startPortalPos = m_Center + Vector2{ -5.f + (idx - 90), 5 };
						}
					}
					else
						TryToMakePortal(isMakingPortal, myIdx, myIdx + 10, startPortalPos, m_Center + Vector2{ -5.f + (idx - 90), 5 }, sectorPtrs);
				}
				else
					TryToMakePortal(isMakingPortal, myIdx, myIdx + 10, startPortalPos, m_Center + Vector2{ -5.f + (idx - 90), 5 }, sectorPtrs);
			}
			TryToMakePortal(isMakingPortal, myIdx, myIdx + 10, startPortalPos, m_Center + Vector2{ 5, 5 }, sectorPtrs);
		}
	}

	{	// RIGHT SIDE

	//Check if this sector has a sector to the right
		if (myIdx % 10 != 9)
		{
			bool isMakingPortal{ false };
			Vector2 startPortalPos{};
			//for the right column
			for (size_t idx{ 9 }; idx < 100; idx += 10)
			{
				//Check if there is a wall at the idx
				if (m_CostField[idx] <= 200)
				{
					//Check if there is a wall on the other side
					if (sectorPtrs[myIdx + 1]->IsWall(idx - 9) == false)
					{
						//MAKE PORTAL
						//Start of portal?
						if (isMakingPortal == false)
						{
							isMakingPortal = true;
							startPortalPos = m_Center + Vector2{ 5, -5.f + idx /10 };
						}
					}
					else
						TryToMakePortal(isMakingPortal, myIdx, myIdx + 1, startPortalPos, m_Center + Vector2{ 5, -5.f + idx / 10 }, sectorPtrs);
				}
				else
					TryToMakePortal(isMakingPortal, myIdx, myIdx + 1, startPortalPos, m_Center + Vector2{ 5, -5.f + idx / 10 }, sectorPtrs);
			}
			TryToMakePortal(isMakingPortal, myIdx, myIdx + 1, startPortalPos, m_Center + Vector2{ 5, 5 }, sectorPtrs);
		}
	}
}

bool Sector::IsWall(int idx) const
{
	return  (m_CostField[idx] >= 200);
}

Vector2 Sector::GetCenter() const
{
	return m_Center;
}

const std::vector<Portal*>* Sector::GetPortals() const
{
	return &m_PortalsPtrs;
}

void Sector::SetHeatFieldPoint(const Elite::Vector2& point, int value)
{
	if (point.x < m_Center.x - 5 || point.x > m_Center.x + 5)
		return;
	if (point.y < m_Center.y - 5 || point.y > m_Center.y + 5)
		return;

	const int idx{ (int(m_Center.x) / 10) + (10 * (int(m_Center.y) / 10)) };

	m_HeatField[idx] = value;
}

void Sector::SetHeatFieldPoint(int idx, int value)
{
	m_HeatField[idx] = value;
}

int Sector::GetHeatFieldValue(int idx) const
{
	return m_HeatField[idx];
}

void Sector::GenerateFlowField()
{
	// dont need to calculate if it has the data already
	if (m_HasFlowFieldGenerated)
		return;

	GenerateHeatField();

	//calculations
	for (int i{}; i < 100; ++i)
	{
		GenerateVectorBasedOnHeatField(i);
	}
}

bool Sector::HasGeneratedFlowField() const
{
	return m_HasFlowFieldGenerated;
}

void Sector::Make1Portal(int myIdx, int otherSectorIdx, const Elite::Vector2& startPortalPos, const Elite::Vector2& endPortalPos)
{
	m_PortalsPtrs.push_back(new Portal(startPortalPos, endPortalPos, myIdx, otherSectorIdx));
}

void Sector::TryToMakePortal(bool& IsMakingPortal, int myIdx, int otherSectorIdx,const Elite::Vector2& startPortalPos, const Elite::Vector2& endPortalPos, const std::vector<Sector*>& sectorPtrs)
{
	if (IsMakingPortal == false)
		return;


	Make1Portal(myIdx, otherSectorIdx, startPortalPos, endPortalPos);
	sectorPtrs[otherSectorIdx]->Make1Portal(otherSectorIdx, myIdx, startPortalPos, endPortalPos);

	IsMakingPortal = false;
}

void Sector::GenerateHeatField()
{
	std::vector<int> toDoList{};
	std::vector<int> finishedList{};

	for (int idx{}; idx < m_HeatField.size(); ++idx)
	{
		if (m_HeatField[idx] != -1)
			toDoList.push_back(idx);
	}

	//For as long as there are nodes that have not been calculated
	while (toDoList.empty() == false)
	{
		std::vector<int> newToDoList{};

		//for every node in to do list
		for (int node : toDoList)
		{

			//Check their connections to set up future caluclation
			for (const int connectedNode : GetCellNeighbors(node))
			{
				bool IsAddedAlready{ false };

				//Check if it is in toDoList
				for (const int todoNode : toDoList)
				{
					if (todoNode == connectedNode)
					{
						IsAddedAlready = true;
						break;
					}
				}
				//Check if it is in newToDoList
				for (const int newTodoNode : newToDoList)
				{
					if (newTodoNode == connectedNode || IsAddedAlready)
					{
						IsAddedAlready = true;
						break;
					}
				}
				//Check if it is in finishedList
				for (const int finishedNode : finishedList)
				{
					if (finishedNode == connectedNode || IsAddedAlready)
					{
						IsAddedAlready = true;
						break;
					}
				}
				//Check if its not a wall
				if (m_CostField[connectedNode] > 200)
				{
					IsAddedAlready = true;
				}

				//if it is in neither, add it to newToDoList
				if (IsAddedAlready == false)
				{
					//Set distance
					m_HeatField[connectedNode] = m_HeatField[node] + 1;

					newToDoList.push_back(connectedNode);
				}
			}
			finishedList.push_back(node);
		}

		toDoList = newToDoList; 
	}
	m_HasFlowFieldGenerated = true;
	FlowFieldFlowOverToNeighborSector();
	
}

std::vector<int> Sector::GetCellNeighbors(int idx)
{
	std::vector<int> returnVector{};

	if (idx < 90)
		returnVector.push_back(idx + 10);
	if (idx > 9)
		returnVector.push_back(idx - 10);
	if (idx % 10 != 9)
		returnVector.push_back(idx + 1);
	if (idx % 10 != 0)
		returnVector.push_back(idx - 1);

	return returnVector;
}

void Sector::FlowFieldFlowOverToNeighborSector()
{
	std::vector<Sector*> neighborSectors{};
	neighborSectors.resize(4);

	int myIdx{ int(m_Center.x) / 10 + 10 * (int(m_Center.y) / 10) };

	if (myIdx < 90)
		neighborSectors[0] = (*m_pSectors)[myIdx + 10];
	if (myIdx > 9)
		neighborSectors[1] = (*m_pSectors)[myIdx - 10];
	if (myIdx % 10 != 9)
		neighborSectors[2] = (*m_pSectors)[myIdx + 1];
	if (myIdx % 10 != 0)
		neighborSectors[3] = (*m_pSectors)[myIdx - 1];


	if (neighborSectors[0])
	{
		if (neighborSectors[0]->HasGeneratedFlowField() == false)
		{
			for (int i{}; i < 10; ++i)
			{
				if (neighborSectors[0]->GetHeatFieldValue(i) == -1 && neighborSectors[0]->IsWall(i) == false
					&& m_HeatField[90 + i] != -1 && m_CostField[90 + i] < 200)
				{
					neighborSectors[0]->SetHeatFieldPoint(i, m_HeatField[90 + i] + 1);
				}
			}
		}
	}
	if (neighborSectors[1])
	{
		if (neighborSectors[1]->HasGeneratedFlowField() == false)
		{
			for (int i{}; i < 10; ++i)
			{
				if (neighborSectors[1]->GetHeatFieldValue(90 + i) == -1 && neighborSectors[1]->IsWall(90 + i) == false
					&& m_HeatField[i] != -1 && m_CostField[i] < 200)
				{
					neighborSectors[1]->SetHeatFieldPoint(90 + i, m_HeatField[i] + 1);
				}
			}
		}
	}
	if (neighborSectors[2])
	{
		if (neighborSectors[2]->HasGeneratedFlowField() == false)
		{
			for (int i{}; i < 10; ++i)
			{
				if (neighborSectors[2]->GetHeatFieldValue(i * 10) == -1 && neighborSectors[2]->IsWall(i * 10) == false
					&& m_HeatField[9 + i * 10] != -1 && m_CostField[9 + i * 10] < 200)
				{
					neighborSectors[2]->SetHeatFieldPoint(i * 10, m_HeatField[9 + i * 10] + 1);
				}
			}
		}
	}
	if (neighborSectors[3])
	{
		if (neighborSectors[3]->HasGeneratedFlowField() == false)
		{
			for (int i{}; i < 10; ++i)
			{
				if (neighborSectors[3]->GetHeatFieldValue(9 + i * 10) == -1 && neighborSectors[3]->IsWall(9 + i * 10) == false
					&& m_HeatField[i * 10] != -1 && m_CostField[i * 10] < 200)
				{
					neighborSectors[3]->SetHeatFieldPoint(9 + i * 10, m_HeatField[i * 10] + 1);
				}
			}
		}
	}
}

void Sector::GenerateVectorBasedOnHeatField(int idx)
{

}
