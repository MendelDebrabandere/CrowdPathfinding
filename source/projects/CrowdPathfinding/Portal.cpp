#include "stdafx.h"
#include "Portal.h"

Portal::Portal(const Elite::Vector2& pos1, const Elite::Vector2& pos2, int otherSectorIdx)
	:m_Pos1{pos1}
	,m_Pos2{pos2}
	,m_OtherSectorIdx{ otherSectorIdx }
{
}

void Portal::Draw() const
{
	DEBUGRENDERER2D->DrawSegment(m_Pos1 + Elite::Vector2{ 0.1f,0.1f }, m_Pos2 + Elite::Vector2{ 0.1f,0.1f }, Elite::Color(1, 0, 1), 0.9f);
}

std::pair<Elite::Vector2, Elite::Vector2> Portal::GetData()
{
	return { m_Pos1, m_Pos2 };
}
