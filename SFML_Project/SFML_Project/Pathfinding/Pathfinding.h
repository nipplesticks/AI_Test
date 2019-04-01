#pragma once
#include "Tile.h"
#include <vector>
#include "Waypoint.h"
#include "QuadTree/QuadTree.h"

class Pathfinding
{
public:
	enum Pathfinding_Heuristic
	{
		Pure_Distance = 0,
		Manhattan_Distance,
		Stanford_Distance
	};

	static bool Flag_Best_Grid_Path;
	static bool Flag_Best_Waypoint_Path;
	static Pathfinding_Heuristic Flag_Pathfinding_Heuristic;
	static bool Flag_Use_Waypoint_Traversal;

public:
	Pathfinding();
	~Pathfinding();

	void Create(const sf::Vector2i & size, const sf::Vector2f & gridStartPosition, const sf::Vector2f & tileSize);

	std::vector<sf::Vector2f> FindPath(const sf::Vector2f & source, const sf::Vector2f & destination);

	void Block(const sf::Vector2i & coord);

	void SetWaypoints(const std::vector<Waypoint> & waypoints, QuadTree * q);

	Tile TileFromWorldCoords(const sf::Vector2f & worldCoord) const;

	const Tile & At(int x, int y);


private:
	struct Node
	{
		Node()
		{
			parentIndex = -1;
			gridTileIndex = 0;
			fCost = FLT_MAX;
			gCost = FLT_MAX;
			hCost = FLT_MAX;
		}
		Node(int _parentIndex, int _gridTileTindex, float _gCost, float _hCost)
		{
			parentIndex = _parentIndex;
			gridTileIndex = _gridTileTindex;
			fCost = _gCost + _hCost;
			gCost = _gCost;
			hCost = _hCost;

		}

		bool operator<(const Node & other) const
		{
			return fCost < other.fCost;
		}

		int parentIndex = -1;
		int gridTileIndex = 0;



		float fCost = FLT_MAX, gCost = FLT_MAX, hCost = FLT_MAX;
	};

	struct WpNode
	{
		WpNode()
		{
			parentIndex = -1;
			ptr = nullptr;
			fCost = FLT_MAX;
			gCost = FLT_MAX;
			hCost = FLT_MAX;
		}
		WpNode(int _parentIndex, Waypoint * _current, float _gCost, float _hCost)
		{
			parentIndex = _parentIndex;
			ptr = _current;
			gCost = _gCost;
			hCost = _hCost;
			fCost = gCost + hCost;
			visitedConnections = std::vector<bool>(ptr->GetConnections().size());
		}
		WpNode(const WpNode & other)
		{
			_copy(other);
		}
		void operator=(const WpNode & other)
		{
			_copy(other);
		}
		bool operator<(const WpNode & other) const
		{
			return fCost < other.fCost;
		}
		void _copy(const WpNode & other)
		{
			parentIndex = other.parentIndex;
			ptr = other.ptr;
			gCost = other.gCost;
			hCost = other.hCost;
			fCost = other.fCost;
			visitedConnections = other.visitedConnections;
		}

		int parentIndex;
		Waypoint * ptr;
		std::vector<bool> visitedConnections;
		float fCost = FLT_MAX, gCost = FLT_MAX, hCost = FLT_MAX;
	};

private:
	void _createFields(int start, int end, QuadTree * q);


private:
	std::vector<Tile> m_grid;
	std::vector<Waypoint> m_waypoints;
	std::vector<WpNode> m_wpNodes;
	sf::Vector2i m_gridSize;


	void _checkNode(const Node & current,
		float addedGCost, int offsetX, int offsetY,
		const Tile & destination,
		std::vector<Node> & openList,
		int parentIndex,
		std::vector<bool> & closedList
	);

	std::vector<Tile> _findPath(const sf::Vector2f & source, const sf::Vector2f & destination);
	bool _isValid(const Tile & tile);
	bool _isValidCoord(const sf::Vector2i & coord);

	float _calcHValue(const Tile & s, const Tile & d);

	// Waypoint tracing
	void _createTileChain(std::vector<Tile> & tileChain, const sf::Vector2f & source, const sf::Vector2f & destination);
	std::vector<WpNode> _findWaypointPath(Waypoint * source, Waypoint * destination, std::vector<WpNode> nodes);

	float _calcWaypointHeuristic(const Waypoint * source, const Waypoint * destination);
};