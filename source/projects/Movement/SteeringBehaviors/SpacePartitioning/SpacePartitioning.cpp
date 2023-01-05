#include "stdafx.h"
#include "SpacePartitioning.h"
#include "projects\Movement\SteeringBehaviors\SteeringAgent.h"
#include <iostream>

// --- Cell ---
// ------------
Cell::Cell()
{
	std::cout << "Cell default constructor\n";
}

Cell::Cell(float left, float bottom, float width, float height)
{
	boundingBox.bottomLeft = { left, bottom };
	boundingBox.width = width;
	boundingBox.height = height;
}

std::vector<Elite::Vector2> Cell::GetRectPoints() const
{
	auto left = boundingBox.bottomLeft.x;
	auto bottom = boundingBox.bottomLeft.y;
	auto width = boundingBox.width;
	auto height = boundingBox.height;

	std::vector<Elite::Vector2> rectPoints =
	{
		{ left , bottom  },
		{ left , bottom + height  },
		{ left + width , bottom + height },
		{ left + width , bottom  },
	};

	return rectPoints;
}

// --- Partitioned Space ---
// -------------------------
CellSpace::CellSpace(float width, float height, int rows, int cols, int maxEntities)
	: m_SpaceWidth(width)
	, m_SpaceHeight(height)
	, m_NrOfRows(rows)
	, m_NrOfCols(cols)
	, m_Neighbors(maxEntities)
	, m_NrOfNeighbors(0)
{
	m_CellWidth = width / float(cols);
	m_CellHeight = height / float(rows);

	for (int idx{}; idx < m_NrOfRows * m_NrOfCols; ++idx)
	{
		m_Cells.push_back(Cell{ (idx % cols) * m_CellWidth,
								(idx / cols) * m_CellHeight,
								m_CellWidth,
								m_CellHeight });
	}
}

void CellSpace::AddAgent(SteeringAgent* agent)
{
	m_Cells[PositionToIndex(agent->GetPosition())].agents.push_back(agent);
}

void CellSpace::UpdateAgentCell(SteeringAgent* agent, Elite::Vector2 oldPos)
{
	int newIdx{ PositionToIndex(agent->GetPosition()) };
	int oldIdx{ PositionToIndex(oldPos) };

	if (newIdx != oldIdx)
	{
		m_Cells[oldIdx].agents.remove(agent);

		m_Cells[newIdx].agents.push_back(agent);
	}

}

void CellSpace::RegisterNeighbors(SteeringAgent* agent, float queryRadius, bool debug)
{
	m_Neighbors.clear();
	m_NrOfNeighbors = 0;

	Elite::Vector2 bottomLeftPos{ agent->GetPosition().x - queryRadius, agent->GetPosition().y - queryRadius };
	queryRadius *= 2;

	int topLeft{ PositionToIndex(bottomLeftPos + Elite::Vector2{0, queryRadius}) };
	int topRight{ PositionToIndex(bottomLeftPos + Elite::Vector2{queryRadius, queryRadius}) };
	int bottomLeft{ PositionToIndex(bottomLeftPos)};
	int bottomRight{ PositionToIndex(bottomLeftPos + Elite::Vector2{queryRadius, 0}) };

	// (infinite loop)
	for (int idx{}; idx < 1000000; ++idx)
	{
		if (debug && agent->CanRenderBehavior())
		{
			Elite::Polygon polygon{ m_Cells[topLeft + idx].GetRectPoints()};
			DEBUGRENDERER2D->DrawPolygon(&polygon, Elite::Color{ 0,1,0 });
		}


		for (auto pAgent : m_Cells[topLeft + idx].agents)
		{
			if ((pAgent->GetPosition() - agent->GetPosition()).Magnitude() <= queryRadius/2)
			{
				m_Neighbors.push_back(pAgent);
				++m_NrOfNeighbors;
				if (agent->CanRenderBehavior())
				{
					DEBUGRENDERER2D->DrawCircle(pAgent->GetPosition(), 1, Elite::Color{ 0,1,0 }, 0.9f);
					//pAgent->SetBodyColor(Elite::Color(0, 1, 0));
				}
			}
		}

		if (topLeft + idx == bottomRight)
			break;
		if (topLeft + idx == topRight)
		{
			topLeft -= m_NrOfCols;
			topRight -= m_NrOfCols;
			idx = -1;
			continue;
		}
	}
}

void CellSpace::EmptyCells()
{
	for (Cell& c : m_Cells)
		c.agents.clear();
}

void CellSpace::RenderCells() const
{
	for (int idx{}; idx < m_NrOfCols; idx++)
	{
		DEBUGRENDERER2D->DrawDirection(	Elite::Vector2{ idx * m_CellWidth, 0 },
										Elite::Vector2{ 0, 1 },
										m_SpaceHeight,
										Elite::Color{ 230,230,250 },
										0.9f);
	}
	for (int idx{}; idx < m_NrOfRows; idx++)
	{
		DEBUGRENDERER2D->DrawDirection(	Elite::Vector2{ 0, idx * m_CellHeight },
										Elite::Vector2{ 1, 0 },
										m_SpaceWidth,
										Elite::Color{ 230,230,250 },
										0.9f);
	}
	for (const Cell& cell : m_Cells)
	{
		int number{ int(cell.agents.size()) };
		Elite::Vector2 pos{ cell.boundingBox.bottomLeft.x, cell.boundingBox.bottomLeft.y + cell.boundingBox.height };

		DEBUGRENDERER2D->DrawString(pos, std::to_string(number).c_str());
	}
}

int CellSpace::PositionToIndex(const Elite::Vector2 pos) const
{
	for (UINT idx{}; idx < m_Cells.size(); ++idx)
	{
		Elite::Rect rect{ m_Cells[idx].boundingBox};

		if (pos.x >= rect.bottomLeft.x && pos.x <= rect.bottomLeft.x + rect.width)
		{
			if (pos.y >= rect.bottomLeft.y && pos.y <= rect.bottomLeft.y + rect.height)
			{
				return idx;
			}
		}
	}

	//bottomLeft
	if (pos.x <= 0 && pos.y <= 0)
		return 0;
	//bottomRight
	else if (pos.x >= m_SpaceWidth && pos.y <= 0)
		return m_NrOfCols - 1;
	//topLeft
	else if (pos.x <= 0 && pos.y >= m_SpaceHeight)
		return m_Cells.size() - m_NrOfCols;
	//topRight
	else if (pos.x >= m_SpaceWidth && pos.y >= m_SpaceHeight)
		return m_Cells.size() - 1;


	// point is out of bounds horizontal
	if (pos.x <= 0 || pos.x >= m_SpaceWidth)
	{
		for (UINT idx{}; idx < m_Cells.size(); ++idx)
		{
			Elite::Rect rect{ m_Cells[idx].boundingBox };
			if (pos.y >= rect.bottomLeft.y && pos.y <= rect.bottomLeft.y + rect.height)
			{
				if (pos.x <= 0)
				{
					return idx;
				}
				else
				{
					return idx + m_NrOfCols - 1;
				}
			}
		}
	}

	// point is out of bounds vertical
	if (pos.y <= 0 || pos.y >= m_SpaceHeight)
	{
		for (UINT idx{}; idx < m_Cells.size(); ++idx)
		{
			Elite::Rect rect{ m_Cells[idx].boundingBox };
			if (pos.x >= rect.bottomLeft.x && pos.x <= rect.bottomLeft.x + rect.width)
			{
				if (pos.y <= 0)
				{
					return idx;
				}
				else
				{
					return idx + m_NrOfCols * (m_NrOfRows-1);
				}
			}
		}
	}
	return -1;
}