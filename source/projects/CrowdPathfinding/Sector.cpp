#include "stdafx.h"
#include "Sector.h"
#include "Portal.h"

using namespace Elite;


Sector::Sector(const Elite::Vector2& center, std::vector<uint8>* costField)
	:m_Center{ center }
	, m_CostField{ costField }
	, m_IntegrationField{}
	, m_FlowField{}
{

	//Create Rigidbodies
	m_RBptrs.resize(costField->size());
	const RigidBodyDefine define = RigidBodyDefine(0.01f, 0.1f, eStatic, false);
	for (int i{}; i < costField->size(); ++i)
	{
		if ((*costField)[i] == 255)
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
}

Sector::~Sector()
{
	SAFE_DELETE(m_CostField);
	for (int i{}; i < m_RBptrs.size(); ++i)
	{
		SAFE_DELETE(m_RBptrs[i]);
	}

	for (int i{}; i < m_PortalsPtrs.size(); ++i)
	{
		SAFE_DELETE(m_PortalsPtrs[i]);
	}

}

void Sector::Draw(bool drawEdged, bool drawCells, bool showPortals) const
{
	constexpr float halfCells{ s_Cells / 2.f };
	if (drawEdged)
	{
		const std::vector<Vector2> polygon{ m_Center + Vector2{-halfCells, -halfCells},
									m_Center + Vector2{-halfCells, halfCells},
									m_Center + Vector2{halfCells, halfCells},
									m_Center + Vector2{halfCells, -halfCells} };
		DEBUGRENDERER2D->DrawPolygon(polygon.data(), 4, Color{ 0.0f,0.0f,0.6f }, 0.7f);
	}
	if (drawCells)
	{
		for (uint8 idx{}; idx < m_CostField->size(); ++idx)
		{
			const std::vector<Vector2> polygon{ m_Center + Vector2{float(-halfCells + idx % s_Cells), float(-halfCells + idx / s_Cells)},
												m_Center + Vector2{float(-halfCells + idx % s_Cells), float(-halfCells + idx / s_Cells + s_CellSize)},
												m_Center + Vector2{float(-halfCells + idx % s_Cells + s_CellSize), float(-halfCells + idx / s_Cells + s_CellSize)},
												m_Center + Vector2{float(-halfCells + idx % s_Cells + s_CellSize), float(-halfCells + idx / s_Cells)} };

			if ((*m_CostField)[idx] == 255)
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
			for (size_t idx{ m_CostField->size() - s_Cells }; idx < m_CostField->size(); ++idx)
			{
				//Check if there is a wall at the idx
				if ((*m_CostField)[idx] != 255)
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
				if ((*m_CostField)[idx] != 255)
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
	return  ((*m_CostField)[idx] == 255);
}

Vector2 Sector::GetCenter() const
{
	return m_Center;
}

void Sector::Make1Portal(int otherSectorIdx, const Elite::Vector2& startPortalPos, const Elite::Vector2& endPortalPos)
{
	m_PortalsPtrs.push_back(new Portal(startPortalPos, endPortalPos, otherSectorIdx));
}

void Sector::TryToMakePortal(bool& IsMakingPortal, int myIdx, int otherSectorIdx,const Elite::Vector2& startPortalPos, const Elite::Vector2& endPortalPos, const std::vector<Sector*>& sectorPtrs)
{
	if (IsMakingPortal == false)
		return;

	Make1Portal(otherSectorIdx, startPortalPos, endPortalPos);
	sectorPtrs[otherSectorIdx]->Make1Portal(myIdx, startPortalPos, endPortalPos);

	IsMakingPortal = false;
}
