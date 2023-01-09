#include "stdafx.h"
#include "Portal.h"

Portal::Portal(const Elite::Vector2& pos1, const Elite::Vector2& pos2)
	:m_Pos1{pos1}
	,m_Pos2{pos2}
{
}

std::pair<Elite::Vector2, Elite::Vector2> Portal::GetData()
{
	return std::pair<Elite::Vector2, Elite::Vector2>(m_Pos1, m_Pos2);
}
