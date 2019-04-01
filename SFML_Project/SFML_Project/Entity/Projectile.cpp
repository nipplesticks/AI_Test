#include "Projectile.h"
#include "Character.h"
#include "AI/Enemy.h"

std::map<unsigned long long, Projectile*> Projectile::s_globalAliveProjectiles;
std::vector<Projectile*> Projectile::s_globalDeadProjectiles;
unsigned long long Projectile::s_index = 0;

void Projectile::CleanDeadProjectiles()
{
	ParticleSystem * ps = ParticleSystem::GetInstance();
	size_t dSize = s_globalDeadProjectiles.size();
	for (size_t i = 0; i < dSize; i++)
	{
		s_globalAliveProjectiles.erase(s_globalDeadProjectiles[i]->m_key);
		ps->AbortWork(s_globalDeadProjectiles[i]->m_particleID);
		delete s_globalDeadProjectiles[i];
	}

	s_globalDeadProjectiles.clear();
}

void Projectile::Create(const sf::Vector2f & at, const sf::Vector2f & dir, float timeAlive, float dmg, Entity * owner)
{
	Projectile * p = new Projectile(at.x, at.y);
	p->m_ownerID = owner->GetUniqueID();
	p->m_key = s_index;
	p->m_aliveTime = timeAlive;
	p->m_dir = dir;
	p->m_dmg = dmg;
	p->m_lastPos = at;

	Particles par(1000, true);
	par.SetPosition(at);

	p->m_particleID = ParticleSystem::GetInstance()->SubmitWork(par);

	s_globalAliveProjectiles.insert(std::make_pair(s_index, p));

	s_index++;
}

Projectile::Projectile(float x, float y)
{
	m_projectile.SetColor(sf::Color::Red);
	m_projectile.SetSize(10, 10);
	m_projectile.SetPosition(x, y);
}

Projectile::Projectile(const Projectile & other)
{
	_copy(other);
}

#include <iostream>
void Projectile::Update(double dt, QuadTree * q)
{
	if (m_aliveTime <= 0.0)
	{
		s_globalDeadProjectiles.push_back(this);
	}
	else
	{
		m_projectile.Translate(m_dir * (float)dt);

		sf::Vector2f newPos = m_projectile.GetPosition();
		sf::Vector2f offset = m_projectile.GetSize() * 0.5f;

		std::vector<unsigned long long> avoidID(2);

		avoidID[0] = m_projectile.GetUniqueID();
		avoidID[1] = m_ownerID;

		Entity * e = q->DispatchRay(m_lastPos + offset, newPos + offset, &avoidID);

		m_lastPos = newPos;
		m_aliveTime -= (float)dt;

		if (e)
		{
			Character * c = dynamic_cast<Character*>(e);
			if (c)
			{
				c->AdjustCurrentHealth(-m_dmg);
			}

			m_aliveTime = 0.0;
		}
		ParticleSystem::GetInstance()->At(m_particleID)->SetPosition(newPos + offset);
	}

	

}

void Projectile::Draw(sf::RenderWindow * wnd)
{
	m_projectile.Draw(wnd);
}

void Projectile::SetAsDead()
{
	m_aliveTime = 0.0;
}

Entity * Projectile::GetEntityPtr()
{
	return &m_projectile;
}

Projectile & Projectile::operator=(const Projectile & other)
{
	if (this != &other)
		_copy(other);

	return *this;
}

void Projectile::_copy(const Projectile & other)
{
	m_projectile = other.m_projectile;
	m_ownerID = other.m_ownerID;
	m_dir = other.m_dir;
	m_lastPos = other.m_lastPos;
	m_aliveTime = other.m_aliveTime;
	m_dmg = other.m_dmg;
	m_key = other.m_key;
	m_particleID = other.m_particleID;
}
