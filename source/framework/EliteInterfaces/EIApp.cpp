#include "stdafx.h"
#include "EIApp.h"

void IApp::RenderWorld() const
{
	std::vector<Elite::Vector2> points =
	{
		{ -m_TrimWorldSize,m_TrimWorldSize },
		{ m_TrimWorldSize,m_TrimWorldSize },
		{ m_TrimWorldSize,-m_TrimWorldSize },
		{-m_TrimWorldSize,-m_TrimWorldSize }
	};
	DEBUGRENDERER2D->DrawPolygon(&points[0], 4, { 1,0,0,1 }, 0.4f);
}
