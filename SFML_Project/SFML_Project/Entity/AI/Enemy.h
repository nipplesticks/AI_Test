#pragma once
#include "../Character.h"
#include "../../Utility/Timer.h"
#include "StateMachine.h"
class Pathfinding;
class QuadTree;


class Enemy : public Character
{
private:
	enum EnemyType
	{
		Random = 0,
		Agressive = 1,
		Passive = 2
	};

public:
	Enemy();
	~Enemy() = default;

	static int s_Spawn_Count;
	static int s_Remaining;

	static void SetMapSize(float x, float y);
	static void SetPlayer(Character * player);
	static void SetPathfindingAndQuadTree(Pathfinding * pf, QuadTree * q);
	
	static Character * GetPlayerPtr();

	void Shoot(int inaccuracy = 1);

	void Update(double dt, bool enableAI = true);

private:
	static Character * s_playerPtr;
	static Pathfinding * s_pfPtr;
	static QuadTree * s_qPtr;
	static sf::Vector2f s_mapSize;

	bool m_playerMoved = false;
	sf::Vector2f m_playerPosLastFrame;

	Timer m_shootTimer;
	double m_shoootTime = 0.0;
	double m_shotsPerSecond = 1.0;
	float m_projectileSpeed = 640.0f;
	float m_bulletLifeTime = 8;
	bool m_canShoot = true;
	float m_prcHp = 0.0f;
	
	EnemyType m_enemyType = Random;

	StateMachine<Enemy> m_stateMachine;

	struct StateVars
	{
		Timer changeStateTimer;
		double changeStateTime = 0.0;

		Timer pathTimer;
		double pathTime = 0.0;
		float waitForPath = 0.0f;

		float minDist = 0.0f;
		float maxDist = 0.0f;
	};

	StateVars m_stateVar;

private: // Random State
	void _setupRandom(std::vector<StateMachine<Enemy>::State> & stateVector);
	std::string _nextStateRandom(double dt);
	void _pureRandom(double dt);
	void _closeRandom(double dt);
	void _keepDistanceRandom(double dt);

private: // Agressive State
	void _setupAgressive(std::vector<StateMachine<Enemy>::State> & stateVector);
	std::string _nextStateAgressive(double dt);
	std::string _leaveRestAgressive(double dt);
	void _restAgressive(double dt);
	void _chargeAgressive(double dt);
	void _strafeAgressive(double dt);

private: // Passive State
	void _setupPassive(std::vector<StateMachine<Enemy>::State> & stateVector);
	std::string _nextStatePassive(double dt);

	float _distToPlayer();
	sf::Vector2f _dirFromPlayer();
	sf::Vector2f _crossDirPlayer(int leftOrRight = 0);
};