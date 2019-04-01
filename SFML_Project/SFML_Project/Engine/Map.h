#pragma once
#include "../Entity/Entity.h"

class Pathfinding;

class Map
{
public:
	Map();
	~Map();
	
	void CreateRandom(const sf::Vector2u & minSize, const sf::Vector2u & maxSize, const sf::Vector2f & tileSize = sf::Vector2f(32.0f, 32.0f));

	void SetBlockedTilesFor(Pathfinding * pf);

	std::vector<Entity> * GetObsticles();
	const sf::Vector2u & GetMapSize() const;

	void Draw(sf::RenderWindow * wnd);
private:
	std::vector<Entity> m_obsticles;
	sf::Vector2f m_obsticlesSize;
	sf::Vector2u m_mapSize;

private:
	void _placeMountain(int x, int y);
};