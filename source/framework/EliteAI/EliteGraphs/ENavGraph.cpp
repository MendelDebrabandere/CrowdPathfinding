#include "stdafx.h"
#include "ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"

using namespace Elite;

Elite::NavGraph::NavGraph(const Polygon& contourMesh, float playerRadius = 1.0f) :
	Graph2D(false),
	m_pNavMeshPolygon(nullptr)
{
	//Create the navigation mesh (polygon of navigatable area= Contour - Static Shapes)
	m_pNavMeshPolygon = new Polygon(contourMesh); // Create copy on heap

	//Get all shapes from all static rigidbodies with NavigationCollider flag
	auto vShapes = PHYSICSWORLD->GetAllStaticShapesInWorld(PhysicsFlags::NavigationCollider);

	//Store all children
	for (auto shape : vShapes)
	{
		shape.ExpandShape(playerRadius);
		m_pNavMeshPolygon->AddChild(shape);
	}

	//Triangulate
	m_pNavMeshPolygon->Triangulate();

	//Create the actual graph (nodes & connections) from the navigation mesh
	CreateNavigationGraph();
}

Elite::NavGraph::~NavGraph()
{
	delete m_pNavMeshPolygon; 
	m_pNavMeshPolygon = nullptr;
}

int Elite::NavGraph::GetNodeIdxFromLineIdx(int lineIdx) const
{
	auto nodeIt = std::find_if(m_Nodes.begin(), m_Nodes.end(), [lineIdx](const NavGraphNode* n) { return n->GetLineIndex() == lineIdx; });
	if (nodeIt != m_Nodes.end())
	{
		return (*nodeIt)->GetIndex();
	}

	return invalid_node_index;
}

Elite::Polygon* Elite::NavGraph::GetNavMeshPolygon() const
{
	return m_pNavMeshPolygon;
}

void Elite::NavGraph::CreateNavigationGraph()
{
	int nodeCounter{};
	//1. Go over all the edges of the navigationmesh and create nodes
	for (UINT lineIdx{}; lineIdx < m_pNavMeshPolygon->GetLines().size(); ++lineIdx)
	{
		if (m_pNavMeshPolygon->GetTrianglesFromLineIndex(lineIdx).size() > 1)
		{
			Line* pLine{ m_pNavMeshPolygon->GetLines()[lineIdx] };
			Vector2 lineCenter{ (pLine->p1 + pLine->p2) / 2 };

			NavGraphNode* node{ new NavGraphNode(nodeCounter, lineIdx, lineCenter)};
			++nodeCounter;
			this->AddNode(node);
		}																							   
	}																								   
																									   		
	//2. Create connections now that every node is created											   		
	for (UINT triangleIdx{}; triangleIdx < m_pNavMeshPolygon->GetTriangles().size(); ++triangleIdx)	   		
	{																								   		
		std::vector<int> NodeIdxs{};																   		
																									   		
		for (int i{}; i < 3; ++i )																	   		
		{																							   		
			int lineIdx{ m_pNavMeshPolygon->GetTriangles()[triangleIdx]->metaData.IndexLines[i] };	   		
			if (lineIdx >= 0)																		   		
			{																						   		
				int nodeIdx{ GetNodeIdxFromLineIdx(lineIdx) };										   		
																									   		
				if (nodeIdx >= 0)																	   		
				{																					   		
					NodeIdxs.push_back(nodeIdx);													   		
				}																					   		
			}																						   		
		}																							   		
																									   		
		if (NodeIdxs.size() == 2)																	   		
		{																							   		
			GraphConnection2D* connection{ new GraphConnection2D(NodeIdxs[0], NodeIdxs[1]) };		   		
			this->AddConnection(connection);														   		
		}																							   		
		if (NodeIdxs.size() == 3)																	   		
		{																							   		
			GraphConnection2D* connection1{ new GraphConnection2D(NodeIdxs[0], NodeIdxs[1]) };		   		
			this->AddConnection(connection1);														   		
			GraphConnection2D* connection2{ new GraphConnection2D(NodeIdxs[1], NodeIdxs[2]) };		   		
			this->AddConnection(connection2);														   		
			GraphConnection2D* connection3{ new GraphConnection2D(NodeIdxs[2], NodeIdxs[0]) };		   		
			this->AddConnection(connection3);														   		
		}																							   		
	}																								   		
																									   		
																											
	//3. Set the connections cost to the actual distance
	this->SetConnectionCostsToDistance();
}

