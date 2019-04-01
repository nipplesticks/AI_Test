#pragma once
#include "../Entity/Character.h"
#include "../Entity/Line.h"
#include "../Entity/Projectile.h"
#include "../Entity/AI/Enemy.h"
#include "../Pathfinding/Pathfinding.h"
#include "../Entity/Particles/ParticleSystem.h"
#include "Map.h"
#include <vector>

class Engine
{
public:
	Engine(sf::RenderWindow * wnd);
	~Engine();

	void HandleEvent(const sf::Event & event);
	void ClearEvent();
	void Update(double dt);
	void Draw();

private:
	struct WindowEvents
	{
		bool Scroll = false;
		float ScrollDelta = 0.0f;

		/*
		Possibly more
		*/
	}m_events;

	bool m_startRound = false;

	const sf::Vector2f OBSTICLE_SIZE = sf::Vector2f(32.0f, 32.0f);
	sf::RenderWindow * m_wndPtr = nullptr;
	Camera m_camera;
	Map m_map;

	sf::Vector2i m_mousePosScreen;
	sf::Vector2f m_mousePosWorld;
	sf::Vector2u m_windowSize;

	sf::RectangleShape m_background;

	Character m_player;
	std::vector<Enemy> m_enemies;

	QuadTree m_quadTree;
	Pathfinding m_pathfinding;
	ParticleSystem * m_particleSystemPtr;

	std::vector<Entity*> m_allEntitiesInTree;

private:
	void _setupMap();
	void _rebuildTree();
	void _handleInput(double dt);
	void _handleCamera(double dt);
	void _handlePlayer(double dt);
};