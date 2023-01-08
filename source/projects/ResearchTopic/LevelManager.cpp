#include "stdafx.h"
#include "LevelManager.h"

using namespace Elite;

LevelManager::LevelManager(int size, int sizeCell, GridGraph<GridTerrainNode, GraphConnection>* gridPtr)
	:m_SizeCell{ sizeCell }
	,m_pGridGraph{ gridPtr }
{
	m_WallRBPtrs.resize(size);
}

LevelManager::~LevelManager()
{
	for (int i{}; i < m_WallRBPtrs.size(); ++i)
	{
		SAFE_DELETE(m_WallRBPtrs[i]);
	}
}

void LevelManager::AddWall(int idx)
{
	if (!m_WallRBPtrs[idx])
	{
		//Create Rigidbody
		const RigidBodyDefine define = RigidBodyDefine(0.01f, 0.1f, eStatic, false);
		const Transform transform = Transform(m_pGridGraph->GetNodeWorldPos(idx), ZeroVector2);

		m_WallRBPtrs[idx] = new RigidBody(define, transform);

		//Add shape
		EPhysicsBoxShape shape;
		shape.width = m_SizeCell;
		shape.height = m_SizeCell;
		m_WallRBPtrs[idx]->AddShape(&shape);
	}
}

void Elite::LevelManager::CheckUpdatedWalls()
{
	for (int i{}; i < m_WallRBPtrs.size(); ++i)
	{
		if (m_WallRBPtrs[i] && m_pGridGraph->GetNode(i)->GetTerrainType() != TerrainType::Water)
		{
			SAFE_DELETE(m_WallRBPtrs[i]);
		}
		else if (!m_WallRBPtrs[i] && m_pGridGraph->GetNode(i)->GetTerrainType() == TerrainType::Water)
		{
			AddWall(i);
		}
	}
}

