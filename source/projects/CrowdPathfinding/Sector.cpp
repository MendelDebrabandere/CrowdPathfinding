#include "stdafx.h"
#include "Sector.h"

using namespace Elite;


Sector::Sector(const Elite::Vector2& center, std::vector<uint8>* costField)
	:m_Center{ center }
	, m_CostField{ costField }
	, m_FlowField{}
{

	//Create Rigidbodies
	m_RBptrs.resize(costField->size());
	const RigidBodyDefine define = RigidBodyDefine(0.01f, 0.1f, eStatic, false);
	for (int i{}; i < costField->size(); ++i)
	{
		if ((*costField)[i] == 255)
		{																			
			const Transform transform = Transform(m_Center + Vector2{ -(s_Cells/2.f - s_CellSize/2) + i % s_Cells,
																	(s_Cells / 2.f - s_CellSize / 2) + i / s_Cells }, ZeroVector2);
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

}

void Sector::Draw(const bool drawEdged, const bool drawCells) const
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

}
