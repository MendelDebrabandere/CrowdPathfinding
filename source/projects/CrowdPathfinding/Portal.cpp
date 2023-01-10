#include "stdafx.h"
#include "Portal.h"

Portal::Portal(const Elite::Vector2& pos1, const Elite::Vector2& pos2, int fromSectorIdx, int otherSectorIdx)
	:m_Pos1{pos1}
	,m_Pos2{pos2}
	,m_OtherSectorIdx{ otherSectorIdx }
	, m_FromSectorIdx{ fromSectorIdx }
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

int Portal::GetTo() const
{
	return m_OtherSectorIdx;
}

int Portal::GetFrom() const
{
	return m_FromSectorIdx;
}
