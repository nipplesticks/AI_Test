#include "Pathfinding.h"
#include <DirectXMath.h>
#include <thread>

#define NUMBER_OF_THREADS 8

bool Pathfinding::Flag_Best_Grid_Path = true;
bool Pathfinding::Flag_Best_Waypoint_Path = false;

Pathfinding::Pathfinding_Heuristic Pathfinding::Flag_Pathfinding_Heuristic = Pathfinding::Pure_Distance;

bool Pathfinding::Flag_Use_Waypoint_Traversal = true;

Pathfinding::Pathfinding()
{
}
	

Pathfinding::~Pathfinding()
{
}

void Pathfinding::Create(const sf::Vector2i & size, const sf::Vector2f & PathfindingStartPosition, const sf::Vector2f & tileSize)
{
	Tile::SetTileSize(tileSize);
	m_grid = std::vector<Tile>(size.x * size.y);
	m_gridSize = size;

	for (int y = 0; y < m_gridSize.y; y++)
	{
		for (int x = 0; x < m_gridSize.x; x++)
		{
			int index = x + y * m_gridSize.x;
			m_grid[index].SetGridCoord(x, y);
			sf::Vector2f offset = sf::Vector2f((float)x, (float)y);
			offset.x *= tileSize.x;
			offset.y *= tileSize.y;
			m_grid[index].SetWorldCoord(PathfindingStartPosition + offset);
		}
	}

}

std::vector<sf::Vector2f> Pathfinding::FindPath(const sf::Vector2f & source, const sf::Vector2f & destination)
{
	if (source.x < m_grid.front().GetWorldCoord().x || source.y < m_grid.front().GetWorldCoord().y ||
		destination.x < m_grid.front().GetWorldCoord().x || destination.y < m_grid.front().GetWorldCoord().y ||
		source.x > m_grid.back().GetWorldCoord().x + Tile::GetTileSize().x || source.y > m_grid.back().GetWorldCoord().y + Tile::GetTileSize().y ||
		destination.x > m_grid.back().GetWorldCoord().x + Tile::GetTileSize().x || destination.y > m_grid.back().GetWorldCoord().y + Tile::GetTileSize().y)
		return std::vector<sf::Vector2f>();


	std::vector<Tile> tileChain;
	std::vector<Tile> path;
	
	if (Flag_Use_Waypoint_Traversal)
		_createTileChain(tileChain, source, destination);

	if (tileChain.size() > 1)
	{
		int pathIterations = (int)tileChain.size() - 1;

		for (int i = 0; i < pathIterations; i++)
		{
			std::vector<Tile> partOfPath = _findPath(tileChain[i].GetWorldCoord(), tileChain[i + 1].GetWorldCoord());
			path.insert(path.end(), partOfPath.begin(), partOfPath.end());
		}
	}
	// Not sure if needed
	// Might be an edge case if source and destination belong to the same waypoint
	else
	{
		path = _findPath(source, destination);
	}

	if (path.empty())
		return std::vector<sf::Vector2f>();


	size_t pSize = path.size();
	std::vector<sf::Vector2f> convertedPath(pSize);

	for (size_t i = 0; i < pSize; i++)
	{
		convertedPath[i] = path[i].GetWorldCoord() + Tile::GetTileSize() * 0.5f;
	}

	return convertedPath;
}

void Pathfinding::Block(const sf::Vector2i & coord)
{
	m_grid[coord.x + coord.y * m_gridSize.x].SetPathable(false);
}

void Pathfinding::SetWaypoints(const std::vector<Waypoint>& waypoints, QuadTree * q)
{
	m_waypoints = waypoints;
	m_wpNodes = std::vector<WpNode>(m_waypoints.size());

	for (size_t i = 0; i < m_waypoints.size(); i++)
	{
		m_wpNodes[i] = WpNode(-1, &m_waypoints[i], 0.0f, 0.0f);
	}

	int totalPathfindingSize = (int)m_grid.size();

	int stepSize = totalPathfindingSize / NUMBER_OF_THREADS;
	std::thread threads[NUMBER_OF_THREADS];

	for (int i = 0; i < NUMBER_OF_THREADS; i++)
	{
		int start = stepSize * i;
		int end = start + stepSize;
		if (i == NUMBER_OF_THREADS - 1)
			end = totalPathfindingSize;

		threads[i] = std::thread(&Pathfinding::_createFields, this, start, end, q);
	}

	for (int i = 0; i < NUMBER_OF_THREADS; i++)
		threads[i].join();
}

Tile Pathfinding::TileFromWorldCoords(const sf::Vector2f & worldCoord) const
{
	sf::Vector2f tileSize = Tile::GetTileSize();
	sf::Vector2f PathfindingStart = m_grid[0].GetWorldCoord();
	sf::Vector2f startToSource = worldCoord - PathfindingStart;
	sf::Vector2i PathfindingSourceIndex = sf::Vector2i((int)(startToSource.x / tileSize.x), (int)(startToSource.y / tileSize.y));

	if (PathfindingSourceIndex.x < 0 || PathfindingSourceIndex.x >= m_gridSize.x * m_gridSize.y)
	{
		return Tile();
	}

	return m_grid[PathfindingSourceIndex.x + PathfindingSourceIndex.y * m_gridSize.x];
}

const Tile & Pathfinding::At(int x, int y)
{
	return m_grid[x + y * m_gridSize.x];
}

void Pathfinding::_createFields(int start, int end, QuadTree * q)
{
	using namespace DirectX;

	int size = (int)m_waypoints.size();

	for (int i = start; i < end; i++)
	{
		sf::Vector2f tWorld = m_grid[i].GetWorldCoord() + m_grid[i].GetTileSize() * 0.5f;

		if (m_grid[i].IsPathable())
		{
			if (m_grid[i].GetFieldOwner() == nullptr)
			{
				float distance = FLT_MAX;
				Waypoint * wp = nullptr;

				for (int i = 0; i < size; i++)
				{
					sf::Vector2f wpWorld = m_waypoints[i].GetWorldCoord();
					Entity * e = q->DispatchRay(tWorld, wpWorld);
					if (e == nullptr)
					{
						float tTemp = XMVectorGetX(XMVector2LengthSq(XMVectorSubtract(XMVectorSet(wpWorld.x, wpWorld.y, 0.0f, 0.0f), XMVectorSet(tWorld.x, tWorld.y, 0.0f, 0.0f))));

						if (tTemp < distance)
						{
							distance = tTemp;
							wp = &m_waypoints[i];
						}

					}
				}
				m_grid[i].SetFieldOwner(wp);
			}
		}
	}

}

void Pathfinding::_checkNode(const Node & current,
	float addedGCost, int offsetX, int offsetY,
	const Tile & destination,
	std::vector<Node>& openList,
	int parentIndex,
	std::vector<bool>& closedList)
{
	sf::Vector2i currentIndex = m_grid[current.gridTileIndex].GetGridCoord();

	if ((offsetX || offsetY) && !(offsetX && offsetY))
	{
		Tile nextTile = Tile(currentIndex + sf::Vector2i(offsetX, offsetY));
		int nextTileIndex = nextTile.GetGridCoord().x + nextTile.GetGridCoord().y * m_gridSize.x;


		if (_isValid(nextTile) && !closedList[nextTileIndex] && m_grid[nextTileIndex].IsPathable())
		{
			Node newNode(parentIndex, nextTile.Get1DGridCoord(m_gridSize.x), current.gCost + addedGCost, _calcHValue(nextTile, destination));
			openList.push_back(newNode);

			if (!Flag_Best_Grid_Path)
				closedList[nextTileIndex] = true;
		}
	}
	else
	{
		Tile nextTileXY = Tile(currentIndex + sf::Vector2i(offsetX, offsetY));
		Tile nextTileX = Tile(currentIndex + sf::Vector2i(offsetX, 0));
		Tile nextTileY = Tile(currentIndex + sf::Vector2i(0, offsetY));

		bool canCross = (_isValid(nextTileX) && m_grid[nextTileX.Get1DGridCoord(m_gridSize.x)].IsPathable()) && (_isValid(nextTileY) && m_grid[nextTileY.Get1DGridCoord(m_gridSize.x)].IsPathable());

		int nextTileIndex = nextTileXY.GetGridCoord().x + nextTileXY.GetGridCoord().y * m_gridSize.x;

		if (canCross && _isValid(nextTileXY) && !closedList[nextTileIndex] && m_grid[nextTileIndex].IsPathable())
		{
			Node newNode(parentIndex, nextTileXY.Get1DGridCoord(m_gridSize.x), current.gCost + addedGCost, _calcHValue(nextTileXY, destination));
			openList.push_back(newNode);

			if (!Flag_Best_Grid_Path)
				closedList[nextTileIndex] = true;
		}
	}

}

std::vector<Tile> Pathfinding::_findPath(const sf::Vector2f & source, const sf::Vector2f & destination)
{
	sf::Vector2f tileSize = Tile::GetTileSize();
	sf::Vector2f PathfindingStart = m_grid[0].GetWorldCoord();

	sf::Vector2f startToSource = source - PathfindingStart;
	sf::Vector2f startToEnd = destination - PathfindingStart;

	sf::Vector2i PathfindingSourceIndex = sf::Vector2i((int)(startToSource.x / tileSize.x), (int)(startToSource.y / tileSize.y));
	sf::Vector2i PathfindingDestIndex = sf::Vector2i((int)(startToEnd.x / tileSize.x), (int)(startToEnd.y / tileSize.y));

	if (PathfindingSourceIndex.x	< 0 || PathfindingSourceIndex.x	>= m_gridSize.x ||
		PathfindingSourceIndex.y	< 0 || PathfindingSourceIndex.y	>= m_gridSize.y ||
		PathfindingDestIndex.x		< 0 || PathfindingDestIndex.x		>= m_gridSize.x || 
		PathfindingDestIndex.y		< 0 || PathfindingDestIndex.y		>= m_gridSize.y)
		return std::vector<Tile>();

	Tile tileSource = m_grid[PathfindingSourceIndex.x + PathfindingSourceIndex.y * m_gridSize.x];
	Tile tileDestination = m_grid[PathfindingDestIndex.x + PathfindingDestIndex.y * m_gridSize.x];

	Node nSource(-1, tileSource.Get1DGridCoord(m_gridSize.x), 0.f, _calcHValue(tileSource, tileDestination));

	if (tileSource.GetSubGrid() != tileDestination.GetSubGrid() || !tileDestination.IsPathable() || !tileSource.IsPathable())
		return std::vector<Tile>();

	std::vector<bool>	closedList(m_gridSize.x * m_gridSize.y);

	std::vector<Node>	openList;
	std::vector<Node>	earlyExploration;
	std::vector<Node> nodes;

	Node	currentNode = nSource;
	Node	earlyExplorationNode;

	openList.push_back(currentNode);

	while (!openList.empty() || earlyExplorationNode.parentIndex != -1)
	{
		if (earlyExplorationNode.parentIndex != -1)
		{
			currentNode = earlyExplorationNode;
			earlyExplorationNode.parentIndex = -1;
		}
		else
		{
			std::sort(openList.begin(), openList.end());
			currentNode = openList.front();
			openList.erase(openList.begin());
		}
		nodes.push_back(currentNode);

		if (currentNode.gridTileIndex == tileDestination.Get1DGridCoord(m_gridSize.x))
		{
			std::vector<Tile> path;

			while (currentNode.parentIndex != -1)
			{
				Tile t = m_grid[currentNode.gridTileIndex];
				path.push_back(t);
				currentNode = nodes[currentNode.parentIndex];
			}

			std::reverse(path.begin(), path.end());
			path.push_back(tileDestination);

			return path;
		}
		closedList[currentNode.gridTileIndex] = true;
		/*
			Generate all the eight successors of the cell
			  N.W	N	N.E
				\	|	/
				 \	|  /
			W --- Node --- E
				 /	|  \
				/	|	\
			  S.W	S	S.E
			Node--> Current Node	= (0, 0)
			N	--> North			= (0, -1)
			S	--> South			= (0, 1)
			W	--> West			= (-1, 0)
			E	--> East			= (1, 0)
			N.W	--> Northwest		= (-1, -1)
			N.E	--> Northeast		= (1, -1)
			S.W	-->	Southwest		= (-1, 1)
			S.E	-->	Southeast		= (1, 1)
		*/

		// AddedGCost based on the distance to the node, 1.0 for direct paths and 1.414 for diagonal paths.
		// Offset defined by the new tiles direction standing at the source tile.
		// Check all possible node directions to see if they are valid or not for further exploration.
		/*---------- North ----------*/

		//float horVerCost = Tile::GetTileSize().x;
		//float diagCost = std::sqrt(Tile::GetTileSize().x * 2.0f);
		float horVerCost = 1.0f;
		float diagCost = 1.414f;

		int parentIndex = (int)nodes.size() - 1;

		_checkNode(currentNode, horVerCost, 0, -1, tileDestination, earlyExploration, parentIndex, closedList);
		/*---------- South ----------*/
		_checkNode(currentNode, horVerCost, 0, 1, tileDestination, earlyExploration, parentIndex, closedList);
		/*---------- West ----------*/
		_checkNode(currentNode, horVerCost, -1, 0, tileDestination, earlyExploration, parentIndex, closedList);
		/*---------- East ----------*/
		_checkNode(currentNode, horVerCost, 1, 0, tileDestination, earlyExploration, parentIndex, closedList);
		/*---------- Northwest ----------*/
		_checkNode(currentNode, diagCost, -1, -1, tileDestination, earlyExploration, parentIndex, closedList);
		/*---------- Northeast ----------*/
		_checkNode(currentNode, diagCost, 1, -1, tileDestination, earlyExploration, parentIndex, closedList);
		/*---------- Southwest ----------*/
		_checkNode(currentNode, diagCost, -1, 1, tileDestination, earlyExploration, parentIndex, closedList);
		/*---------- Southeast ----------*/
		_checkNode(currentNode, diagCost, 1, 1, tileDestination, earlyExploration, parentIndex, closedList);

		std::sort(earlyExploration.begin(), earlyExploration.end());

		if (!earlyExploration.empty() && earlyExploration.front().fCost <= currentNode.fCost)
		{
			earlyExplorationNode = earlyExploration.front();
			earlyExploration.erase(earlyExploration.begin());
		}
		openList.insert(openList.end(), earlyExploration.begin(), earlyExploration.end());
		earlyExploration.clear();
	}

	//pathFile.close();

	return std::vector<Tile>();
}

bool Pathfinding::_isValid(const Tile & tile)
{
	sf::Vector2i index = tile.GetGridCoord();

	if (index.x < 0 || index.x >= m_gridSize.x || index.y < 0 || index.y >= m_gridSize.y)
		return false;

	return true;
}

bool Pathfinding::_isValidCoord(const sf::Vector2i & coord)
{
	if (coord.x < 0 || coord.x >= m_gridSize.x || coord.y < 0 || coord.y >= m_gridSize.y)
		return false;
	return true;
}

float Pathfinding::_calcHValue(const Tile & s, const Tile & d)
{
	using namespace DirectX;

	auto p1 = s.GetGridCoord();
	auto p2 = d.GetGridCoord();

	auto deltaX = abs(p1.x - p2.x);
	auto deltaY = abs(p1.y - p2.y);

	switch (Flag_Pathfinding_Heuristic)
	{
	case Pathfinding::Pure_Distance:
		return XMVectorGetX(XMVector2LengthSq(XMVectorSubtract(XMVectorSet((float)p2.x, (float)p2.y, 0.0f, 0.0f), XMVectorSet((float)p1.x, (float)p1.y, 0.0f, 0.0f))));;
		break;
	case Pathfinding::Manhattan_Distance:
		return float(deltaX + deltaY);
		break;
	case Pathfinding::Stanford_Distance:
		return float(deltaX + deltaY) + (-0.414f) * (float)std::min(deltaX, deltaY);
		break;
	default:
		return 2.0f;
		break;
	}
}

void Pathfinding::_createTileChain(std::vector<Tile>& tileChain, const sf::Vector2f & source, const sf::Vector2f & destination)
{
	Tile sourceTile = TileFromWorldCoords(source);
	Tile destinationTile = TileFromWorldCoords(destination);

	Waypoint * sourceWaypoint = sourceTile.GetFieldOwner();
	Waypoint * destinationWaypoint = destinationTile.GetFieldOwner();

	if (sourceWaypoint == nullptr || destinationWaypoint == nullptr)
		return;

	std::vector<WpNode> waypointPath = _findWaypointPath(sourceWaypoint, destinationWaypoint, m_wpNodes);
	
	tileChain.push_back(TileFromWorldCoords(source));

	if (!waypointPath.empty())
	{
		int nrOfWaypoints = (int)waypointPath.size();
		for (int i = 0; i < nrOfWaypoints; i++)
		{
			tileChain.push_back(TileFromWorldCoords(waypointPath[i].ptr->GetWorldCoord()));
		}
	}

	tileChain.push_back(TileFromWorldCoords(destination));
}

std::vector<Pathfinding::WpNode> Pathfinding::_findWaypointPath(Waypoint * source, Waypoint * destination, std::vector<WpNode> nodes)
{
	WpNode current = WpNode(-1, source, 0.f, _calcWaypointHeuristic(source, destination));
	std::vector<WpNode> openList;
	std::vector<WpNode> waypointNodes;
	std::vector<WpNode> earlyExploration;

	openList.push_back(current);

	while (!openList.empty())
	{
		std::sort(openList.begin(), openList.end());
		current = openList.front();
		openList.erase(openList.begin());
		waypointNodes.push_back(current);

		if (current.ptr == destination)
		{
			std::vector<WpNode> waypointPath;

			waypointPath.push_back(current);
			while (current.parentIndex != -1)
			{
				waypointPath.push_back(waypointNodes[current.parentIndex]);
				current = waypointPath.back();
			}
			std::reverse(waypointPath.begin(), waypointPath.end());

			return waypointPath;
		}

		float lowestConnectionCost = FLT_MAX;
		int goToWaypoint = -1;
		int waypointIndex = 0;
		const std::vector<Waypoint::Connection> & waypointConnections = current.ptr->GetConnections();

		if (!waypointConnections.empty())
		{
			int connectionsSize = (int)waypointConnections.size();

			for (int i = 0; i < connectionsSize; i++)
			{
				if (!nodes[current.ptr->GetArrayIndex()].visitedConnections[i])
				{
					nodes[current.ptr->GetArrayIndex()].visitedConnections[i] = true;
					if (&m_waypoints[waypointConnections[i].Waypoint] == destination)
					{
						earlyExploration.push_back(WpNode((int)waypointNodes.size() - 1, &m_waypoints[waypointConnections[i].Waypoint], 0,0));
						goToWaypoint = waypointConnections[i].Waypoint;
						waypointIndex = i;

					}
					else
					{
						if (Pathfinding::Flag_Best_Waypoint_Path || nodes[m_waypoints[waypointConnections[i].Waypoint].GetArrayIndex()].parentIndex)
						{
							earlyExploration.push_back(WpNode((int)waypointNodes.size() - 1, &m_waypoints[waypointConnections[i].Waypoint],
								waypointConnections[i].Cost + current.gCost,
								_calcWaypointHeuristic(&m_waypoints[waypointConnections[i].Waypoint], destination)));

							nodes[earlyExploration.back().ptr->GetArrayIndex()].parentIndex = 0;

							float connectionTraversalCost = _calcWaypointHeuristic(&m_waypoints[waypointConnections[i].Waypoint], destination) + waypointConnections[i].Cost;
							if (connectionTraversalCost < lowestConnectionCost)
							{
								lowestConnectionCost = connectionTraversalCost;
								goToWaypoint = waypointConnections[i].Waypoint;
								waypointIndex = i;
							}
						}
					}
				}
			}

			if (goToWaypoint != -1)
			{
				//std::sort(earlyExploration.begin(), earlyExploration.end());

				if (Pathfinding::Flag_Best_Waypoint_Path)
				{
					for (int i = 0; i < earlyExploration.size(); i++)
					{
						Waypoint * currentPtr = earlyExploration[i].ptr;
						auto iterator = std::find(currentPtr->GetConnections().begin(), currentPtr->GetConnections().end(), current.ptr);
						int index = (int)(iterator - currentPtr->GetConnections().begin());
						nodes[currentPtr->GetArrayIndex()].visitedConnections[index] = true;
					}
				}

				openList.insert(openList.end(), earlyExploration.begin(), earlyExploration.end());
				earlyExploration.clear();
			}
		}
	}

	return std::vector<WpNode>();
}

float Pathfinding::_calcWaypointHeuristic(const Waypoint * source, const Waypoint * destination)
{
	using namespace DirectX;

	auto p1 = source->GetWorldCoord();
	auto p2 = destination->GetWorldCoord();

	auto deltaX = abs(p1.x - p2.x);
	auto deltaY = abs(p1.y - p2.y);

	switch (Flag_Pathfinding_Heuristic)
	{
	case Pathfinding::Pure_Distance:
		return XMVectorGetX(XMVector2LengthSq(XMVectorSubtract(XMVectorSet(p2.x, p2.y, 0.0f, 0.0f), XMVectorSet(p1.x, p1.y, 0.0f, 0.0f))));;
		break;
	case Pathfinding::Manhattan_Distance:
		return float(deltaX + deltaY);
		break;
	case Pathfinding::Stanford_Distance:
		return float(deltaX + deltaY) + (-0.414f) * (float)std::min(deltaX, deltaY);
		break;
	default:
		return 2.0f;
		break;
	}
}
