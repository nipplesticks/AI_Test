#pragma once
#include "Entity.h"
#include "../Pathfinding/QuadTree/QuadTree.h"
#include <map>
#include "Particles/ParticleSystem.h"

class Projectile
{
public:
	static std::map<unsigned long long, Projectile*> s_globalAliveProjectiles;

	static void CleanDeadProjectiles();

	static void Create(const sf::Vector2f & at, const sf::Vector2f & dir, float timeAlive, float dmg, Entity * owner);

	/* Returns the state if this projectile can be drawn */
	void Update(double dt, QuadTree * q);

	void Draw(sf::RenderWindow * wnd);

	void SetAsDead();

	Entity * GetEntityPtr();

	Projectile & operator=(const Projectile & other);

private:
	Projectile(float x = 0.0f, float y = 0.0f);
	Projectile(const Projectile & other);
	~Projectile() = default;
private:
	static unsigned long long s_index;
	static std::vector<Projectile*> s_globalDeadProjectiles;

	Entity m_projectile;
	unsigned long long m_ownerID;
	sf::Vector2f m_dir;

	ull m_particleID;

	sf::Vector2f m_lastPos;

	float m_aliveTime;
	float m_dmg;
	unsigned long long m_key = -1;

private:
	void _copy(const Projectile & other);
};