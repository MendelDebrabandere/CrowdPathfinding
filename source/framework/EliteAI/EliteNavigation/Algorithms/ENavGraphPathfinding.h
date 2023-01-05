#pragma once
#include <vector>
#include <iostream>
#include "framework/EliteMath/EMath.h"
#include "framework\EliteAI\EliteGraphs\ENavGraph.h"
#include "framework\EliteAI\EliteGraphs\EliteGraphAlgorithms\EAStar.h"
#include "framework/EliteAI/eliteGraphs/eigraph.h"
#include "framework/EliteAI/eliteGraphs/ENavGraph.h"
#include "framework/EliteAI/eliteGraphs/EliteGraphAlgorithms/EAstar.h"

namespace Elite
{
	class NavMeshPathfinding
	{
	public:
		static std::vector<Vector2> FindPath(	Vector2 startPos,
												Vector2 endPos,
												NavGraph* pNavGraph,
												std::vector<Vector2>& debugNodePositions,
												std::vector<Portal>& debugPortals)
		{
			//Create the path to return

			std::vector<Vector2> finalPath{};

			//Get the start and endTriangle
			const Triangle* startTriangle{ pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(startPos) };
			const Triangle* endTriangle{ pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(endPos) };

			if (!startTriangle || !endTriangle)
			{
				return finalPath;
			}
			if (startTriangle == endTriangle)
			{
				finalPath.push_back(endPos);
				return finalPath;
			}

			//We have valid start/end triangles and they are not the same
			//=> Start looking for a path
			//Copy the graph

			auto cloneGraph{ pNavGraph->Clone() };

			//Create extra node for the Start Node (Agent's position
			NavGraphNode* startNode{ new NavGraphNode(cloneGraph->GetNextFreeNodeIndex(), -1, startPos) };
			cloneGraph->AddNode(startNode);
			//const Triangle* startTriangle{ pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(startPos) };
			
			for (int i{}; i < 3; ++i)
			{
				int lineIdx{ startTriangle->metaData.IndexLines[i] };
				int nodeIdx{ pNavGraph->GetNodeIdxFromLineIdx(lineIdx) };
				if (nodeIdx >= 0)
					cloneGraph->AddConnection(new GraphConnection2D(startNode->GetIndex(), nodeIdx, Distance(startPos, pNavGraph->GetNodePos(nodeIdx))));
			}

			//delete startNode;
			//delete startTriangle;

			//Create extra node for the endNode
			NavGraphNode* endNode{ new NavGraphNode(cloneGraph->GetNextFreeNodeIndex(), -1, endPos) };
			cloneGraph->AddNode(endNode);
			//const Triangle* endTriangle{ pNavGraph->GetNavMeshPolygon()->GetTriangleFromPosition(endPos) };


			for (int i{}; i < 3; ++i)
			{
				int endLineIdx{ endTriangle->metaData.IndexLines[i] };
				int endNodeIdx{ pNavGraph->GetNodeIdxFromLineIdx(endLineIdx) };
				if (endNodeIdx >= 0)
					cloneGraph->AddConnection(new GraphConnection2D(endNode->GetIndex(), endNodeIdx, Distance(endPos, pNavGraph->GetNodePos(endNodeIdx))));
			}

			//delete endNode;
			//delete endTriangle;

			//Run A star on new graph
			AStar< NavGraphNode, GraphConnection2D> AstarThing{ cloneGraph.get(),HeuristicFunctions::Chebyshev };
			std::vector<NavGraphNode*> NodePath{ AstarThing.FindPath(startNode, endNode) };

			//OPTIONAL BUT ADVICED: Debug Visualisation

			//Run optimiser on new graph, MAKE SURE the A star path is working properly before starting this section and uncommenting this!!!
			const std::vector<Portal> m_Portals = SSFA::FindPortals(NodePath, pNavGraph->GetNavMeshPolygon());
			finalPath = SSFA::OptimizePortals(m_Portals);

			//for (const NavGraphNode* nodePtr : NodePath)
			//{
			//	finalPath.push_back(nodePtr->GetPosition());
			//}

			return finalPath;
		}
	};
}
