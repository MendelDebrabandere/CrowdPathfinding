#pragma once
#include "framework/EliteAI/EliteNavigation/ENavigation.h"

namespace Elite
{
	template <class T_NodeType, class T_ConnectionType>
	class AStar
	{
	public:
		AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction);

		// stores the optimal connection to a node and its total costs related to the start and end node of the path
		struct NodeRecord
		{
			T_NodeType* pNode = nullptr;
			T_ConnectionType* pConnection = nullptr;
			float costSoFar = 0.f; // accumulated g-costs of all the connections leading up to this one
			float estimatedTotalCost = 0.f; // f-cost (= costSoFar + h-cost)

			bool operator==(const NodeRecord& other) const
			{
				return pNode == other.pNode
					&& pConnection == other.pConnection
					&& costSoFar == other.costSoFar
					&& estimatedTotalCost == other.estimatedTotalCost;
			};

			bool operator<(const NodeRecord& other) const
			{
				return estimatedTotalCost < other.estimatedTotalCost;
			};
		};

		std::vector<T_NodeType*> FindPath(T_NodeType* pStartNode, T_NodeType* pDestinationNode);

	private:
		float GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const;

		IGraph<T_NodeType, T_ConnectionType>* m_pGraph;
		Heuristic m_HeuristicFunction;
	};

	template <class T_NodeType, class T_ConnectionType>
	AStar<T_NodeType, T_ConnectionType>::AStar(IGraph<T_NodeType, T_ConnectionType>* pGraph, Heuristic hFunction)
		: m_pGraph(pGraph)
		, m_HeuristicFunction(hFunction)
	{
	}



	template <class T_NodeType, class T_ConnectionType>
	std::vector<T_NodeType*> AStar<T_NodeType, T_ConnectionType>::FindPath(T_NodeType* pStartNode, T_NodeType* pGoalNode)
	{
		std::vector<T_NodeType*> path;
		std::vector<NodeRecord> openList;
		std::vector<NodeRecord> closedList;

		// 1. Create a NodeRecord to kickstart the loop
		NodeRecord currentRecord{ pStartNode, nullptr, 0.0f, GetHeuristicCost(pStartNode, pGoalNode) };
		openList.push_back(currentRecord);
		
		// 2. While (openlist != empty)
		while (openList.size() != 0)
		{
			// A. Get NodeRecord with lowest costfrom openList
			currentRecord = *std::min_element(openList.begin(), openList.end());

			// B. Check if that connection leads to the end node
			if (currentRecord.pNode == pGoalNode) 
				break;

			// C. Else we get all the connections of the NodeRecor's node
			for (const auto& connection : m_pGraph->GetNodeConnections(currentRecord.pNode))
			{
				// D. Check if any of those connections lead to a node already in the closed list
				T_NodeType* pConnectedNode{ m_pGraph->GetNode(connection->GetTo()) };

				const float currentCost{ currentRecord.costSoFar + connection->GetCost() };

				bool foundCheaper{false};

				for (const NodeRecord& NRecord : closedList)
				{
					if (NRecord.pNode == pConnectedNode)
					{
						if (NRecord.costSoFar < currentCost)
						{
							foundCheaper = true;
							break;
						}

						closedList.erase(std::remove(closedList.begin(), closedList.end(), NRecord));
						break;
					}
				}
				if (foundCheaper) continue;



				// E. If 2.D check failed, check if any of those connections lead to a node already in the open list
				for (const NodeRecord& upcomingRecord : openList)
				{
					if (upcomingRecord.pNode == pConnectedNode)
					{
						if (upcomingRecord.costSoFar < currentCost)
						{
							foundCheaper = true;
							break;
						}
						openList.erase(std::remove(openList.begin(), openList.end(), upcomingRecord));
						break;
					}
				}
				if (foundCheaper) continue;


				// F.	At this point any expensive connection should be removed (if it existed).
				//		We create a new nodeRecord and add it to the openList
				openList.push_back(NodeRecord{ pConnectedNode, connection, currentCost, currentCost + GetHeuristicCost(pConnectedNode, pGoalNode) });
			}

			// G. Remove NodeRecord from the openList and add it to the closedList
			openList.erase(std::remove(openList.begin(), openList.end(), currentRecord));
			closedList.push_back(currentRecord);
		}

		// 3. Reconstruct path from last connection to start node
		while (currentRecord.pNode != pStartNode)
		{
			path.push_back(currentRecord.pNode);

			for (const NodeRecord& Node : closedList)
			{
				if (currentRecord.pConnection->GetFrom() == Node.pNode->GetIndex())
				{
					currentRecord = Node;
					break;
				}
			}
		}


		path.push_back(pStartNode);

		
		std::reverse(path.begin(), path.end());

		return path;
	}

	template <class T_NodeType, class T_ConnectionType>
	float Elite::AStar<T_NodeType, T_ConnectionType>::GetHeuristicCost(T_NodeType* pStartNode, T_NodeType* pEndNode) const
	{
		Vector2 toDestination = m_pGraph->GetNodePos(pEndNode) - m_pGraph->GetNodePos(pStartNode);
		return m_HeuristicFunction(abs(toDestination.x), abs(toDestination.y));
	}
}