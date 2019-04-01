#include "Engine.h"
#include "../Utility/Timer.h"
#include <iostream>
#include <DirectXMath.h>

Engine::Engine(sf::RenderWindow * wnd)
{
	Enemy::SetPlayer(&m_player);
	Enemy::SetPathfindingAndQuadTree(&m_pathfinding, &m_quadTree);
	srand((unsigned)time(0));
	m_wndPtr = wnd;

	m_background.setSize((sf::Vector2f)wnd->getSize());
	m_background.setPosition(0.0f, 0.0f);
	m_background.setFillColor(sf::Color(150, 200, 0));

	m_camera.SetAsActive();

	_setupMap();

	Pathfinding::Flag_Best_Grid_Path = false;
	Pathfinding::Flag_Use_Waypoint_Traversal = false;
	Pathfinding::Flag_Pathfinding_Heuristic = Pathfinding::Pathfinding_Heuristic::Stanford_Distance;

	m_particleSystemPtr = ParticleSystem::GetInstance();

	m_player.SetColor(sf::Color::Red);
}

Engine::~Engine()
{

}

void Engine::HandleEvent(const sf::Event & event)
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape))
		m_wndPtr->close();

	if (event.type == sf::Event::Closed)
		m_wndPtr->close();
	else if (event.type == sf::Event::MouseWheelMoved)
	{
		m_events.Scroll = true;
		m_events.ScrollDelta = event.mouseWheel.delta * -1.0f;
	}
}

void Engine::ClearEvent()
{
	memset(&m_events, 0, sizeof(m_events));
}

void Engine::Update(double dt)
{
	if (m_wndPtr->hasFocus())
	{
		static bool reset = false;

		m_particleSystemPtr->Begin(dt);

		if (reset)
		{
			_setupMap();
			reset = false;
		}

		_rebuildTree();
		_handleInput(dt);

		Projectile::CleanDeadProjectiles();
		for (auto & p : Projectile::s_globalAliveProjectiles)
			p.second->Update(dt, &m_quadTree);

		size_t eSize = m_enemies.size();
		
		for (int i = 0; i < eSize; i++)
		{
			if (!m_enemies[i].IsDead())
			{
				m_enemies[i].Update(dt, m_startRound);
			}
			else
			{
				m_enemies.erase(m_enemies.begin() + i);
				i--;
				eSize--;
				Enemy::s_Remaining--;
			}
		}
		
		m_player.Update(dt);
	
		m_particleSystemPtr->End();
		/*if (m_player.IsDead())
			reset = true;*/

	}
}

void Engine::Draw()
{
	m_wndPtr->draw(m_background);
	m_map.Draw(m_wndPtr);

	for (auto & e : m_enemies)
		e.Draw(m_wndPtr);

	m_player.Draw(m_wndPtr);
	for (auto & p : Projectile::s_globalAliveProjectiles)
		p.second->Draw(m_wndPtr);

	m_particleSystemPtr->Draw(m_wndPtr);
}

void Engine::_setupMap()
{
	m_enemies.clear();
	m_startRound = false;
	for (auto & p : Projectile::s_globalAliveProjectiles)
		p.second->SetAsDead();

	Projectile::CleanDeadProjectiles();

	m_player.SetPosition(32.01f, 32.01f);
	m_player.SetPath();
	m_player.SetCurrentHealth(100);
	m_camera.SetPosition(sf::Vector3f(m_player.GetPosition().x, m_player.GetPosition().y, m_camera.GetPosition().z));

	sf::Vector2u minSize, maxSize;
	minSize.x = 10u;
	minSize.y = 10u;
	maxSize.x = 100u;
	maxSize.y = 100u;

	m_map.CreateRandom(minSize, maxSize, OBSTICLE_SIZE);

	sf::Vector2u mapSize = m_map.GetMapSize();

	Enemy::SetMapSize(mapSize.x * OBSTICLE_SIZE.x, mapSize.y * OBSTICLE_SIZE.y);

	m_quadTree.BuildTree(6, std::max(mapSize.x, mapSize.y) * (unsigned)OBSTICLE_SIZE.x, sf::Vector2f(0, 0));

	m_quadTree.PlaceObjects(*m_map.GetObsticles());

	m_pathfinding.Create((sf::Vector2i)mapSize, sf::Vector2f(0, 0), OBSTICLE_SIZE);

	m_map.SetBlockedTilesFor(&m_pathfinding);

	int numberOfEnemies = rand() % (std::min(mapSize.x, mapSize.y) / 10 + 1) + 1;
	numberOfEnemies = 0;

	Enemy::s_Spawn_Count = numberOfEnemies;
	Enemy::s_Remaining = numberOfEnemies;

	for (int i = 0; i < numberOfEnemies; i++)
	{
		_rebuildTree();
		bool placed = false;
		while (!placed)
		{
			float x = (float)(rand() % (mapSize.x - 1) + 1) * OBSTICLE_SIZE.x;
			float y = (float)(rand() % (mapSize.y - 1) + 1) * OBSTICLE_SIZE.y;

			Entity * e = m_quadTree.PointInsideObject(sf::Vector2f(x + OBSTICLE_SIZE.x * 0.5f, y + OBSTICLE_SIZE.y * 0.5f));

			if (e == nullptr)
			{
				placed = true;
				Enemy en;
				en.SetPosition(x, y);
				en.SetSize(OBSTICLE_SIZE);
				//e.SetColor(sf::Color::Blue);
				m_enemies.push_back(en);
			}
		}
	}

	

	_rebuildTree();
}

void Engine::_rebuildTree()
{
	m_allEntitiesInTree.clear();

	size_t numberOfEnemies = m_enemies.size();

	size_t aSize = m_map.GetObsticles()->size() + numberOfEnemies;
	size_t pSize = Projectile::s_globalAliveProjectiles.size();
	m_allEntitiesInTree = std::vector<Entity*>(aSize + pSize + 1);

	std::vector<Entity> * mapOb = m_map.GetObsticles();

	size_t i;
	for (i = 0; i < aSize - numberOfEnemies; i++)
	{
		m_allEntitiesInTree[i] = &mapOb->at(i);
	}
	for (int e = 0; e < numberOfEnemies; e++)
	{
		m_allEntitiesInTree[i++] = &m_enemies[e];
	}
	for (auto & p : Projectile::s_globalAliveProjectiles)
		m_allEntitiesInTree[i++] = p.second->GetEntityPtr();

	m_allEntitiesInTree[i++] = &m_player;
	
	m_quadTree.PlaceObjects(m_allEntitiesInTree);
}

void Engine::_handleInput(double dt)
{
	static bool s_ResetMap = false;
	
	bool resetPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Tab);

	if (resetPressed && !s_ResetMap)
		_setupMap();

	s_ResetMap = resetPressed;

	m_windowSize = m_wndPtr->getSize();
	m_mousePosScreen = sf::Mouse::getPosition(*m_wndPtr);



	_handleCamera(dt);

	float zoom = m_camera.GetPosition().z;
	m_mousePosWorld.x = (((float)m_mousePosScreen.x - (float)m_windowSize.x * 0.5f) * zoom) + m_camera.GetPosition().x;
	m_mousePosWorld.y = (((float)m_mousePosScreen.y - (float)m_windowSize.y * 0.5f) * zoom) + m_camera.GetPosition().y;

	_handlePlayer(dt);
}

void Engine::_handleCamera(double dt)
{
	static const float CAMERA_SPEED = 500.0f;
	static const float CAMERA_ZOOM = 0.5f;
	static const int MOVE_SENSE_X = 50;
	static const int MOVE_SENSE_Y = 50;

	static float s_TargetScrollVal = 0.0f;
	static float s_CurrentScrollVal = 0.0f;

	if (!sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
	{
		bool cameraMove = false;
		sf::Vector2f camTranslate(0, 0);

		float increment = 1;

		if (m_mousePosScreen.y < MOVE_SENSE_Y)
		{
			cameraMove = true;
			camTranslate.y -= CAMERA_SPEED * (float)dt * increment;
		}
		if (m_mousePosScreen.y > (int)m_windowSize.y - MOVE_SENSE_Y)
		{
			cameraMove = true;
			camTranslate.y += CAMERA_SPEED * (float)dt * increment;
		}
		if (m_mousePosScreen.x > (int)m_windowSize.x - MOVE_SENSE_X)
		{
			cameraMove = true;
			camTranslate.x += CAMERA_SPEED * (float)dt * increment;
		}
		if (m_mousePosScreen.x < MOVE_SENSE_X)
		{
			cameraMove = true;
			camTranslate.x -= CAMERA_SPEED * (float)dt * increment;
		}
		if (cameraMove)
			m_camera.Translate(camTranslate.x, camTranslate.y);
	}
	else
	{
		auto pPos = m_player.GetPosition();
		sf::Vector3f cPos(pPos.x, pPos.y, m_camera.GetPosition().z);
		m_camera.SetPosition(cPos);
	}
	
	if (m_events.Scroll)
	{
		float inc = 1.0f;

		if (s_TargetScrollVal < 0.0f && m_events.ScrollDelta > 0.0f || s_TargetScrollVal > 0.0f && m_events.ScrollDelta < 0.0f)
			inc = 2.0f;

		s_TargetScrollVal += m_events.ScrollDelta * inc;

		s_TargetScrollVal = std::max(s_TargetScrollVal, -15.0f);
		s_TargetScrollVal = std::min(s_TargetScrollVal, 15.0f);

		s_CurrentScrollVal = s_TargetScrollVal;
	}

	if (s_CurrentScrollVal != 0.0f)
	{
		auto pos = m_camera.GetPosition();
		float deltaZoom = s_CurrentScrollVal * ((float)dt / CAMERA_ZOOM);
		pos.z += deltaZoom;

		bool wasNegative = s_CurrentScrollVal < 0.0f;
		float currentVal = s_CurrentScrollVal;

		s_CurrentScrollVal -= s_TargetScrollVal * ((float)dt / CAMERA_ZOOM);

		if ((s_CurrentScrollVal > 0.0f && wasNegative) || (s_CurrentScrollVal < 0.0f && !wasNegative) || fabs(s_CurrentScrollVal) < 0.01f)
		{
			s_CurrentScrollVal = 0.0f;
			s_TargetScrollVal = 0.0f;
		}
		else
		{
			pos.z = std::min(pos.z, 5.0f);
			pos.z = std::max(pos.z, 0.5f);

			m_camera.SetPosition(pos);
			
		}
	}
}

void Engine::_handlePlayer(double dt)
{
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S))
	{
		auto playerPath = m_player.GetPath();
		if (!playerPath.empty())
		{
			playerPath.erase(playerPath.begin() + 1, playerPath.end());
			m_player.SetPath(playerPath);
		}
	}
	else if (sf::Mouse::isButtonPressed(sf::Mouse::Right))
	{
		m_startRound = true;
		auto path = m_pathfinding.FindPath(m_player.GetPosition() + m_player.GetSize() * 0.5f, m_mousePosWorld);

		if (!path.empty())
			m_player.SetPath(path);
	}

	static bool s_canShoot = true;
	static Timer s_shootTimer(true);
	static double s_shootTime = 0.0;

	//bool Shot = sf::Keyboard::isKeyPressed(sf::Keyboard::Q);
	bool Shot = sf::Mouse::isButtonPressed(sf::Mouse::Left);
	using namespace DirectX;
	if (Shot && s_canShoot)
	{
		m_startRound = true;
		s_shootTimer.Stop();
		s_shootTime = 0.0;
		auto pPos = m_player.GetPosition();
		XMVECTOR dir = XMVector2Normalize(XMVectorSubtract(XMLoadFloat2((XMFLOAT2*)&m_mousePosWorld), XMLoadFloat2((XMFLOAT2*)&pPos)));
		float projSpeed = 640.0f;
		dir = XMVectorScale(dir, projSpeed);
		sf::Vector2f sfDir;
		DirectX::XMStoreFloat2(((XMFLOAT2*)&sfDir), dir);		
		Projectile::Create(pPos + m_player.GetSize() * 0.5f, sfDir, 2, 12, &m_player);
		s_canShoot = false;
	}

	if (!s_canShoot)
	{
		s_shootTime += s_shootTimer.Stop();


		if (s_shootTime > 0.1)
		{
			s_canShoot = true;
		}
	}


}
