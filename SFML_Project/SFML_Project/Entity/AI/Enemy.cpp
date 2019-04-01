#include "Enemy.h"
#include "../../Pathfinding/Pathfinding.h"
#include <DirectXMath.h>
#include "../Projectile.h"
#include <iostream>

Character * Enemy::s_playerPtr = nullptr;
Pathfinding * Enemy::s_pfPtr = nullptr;
QuadTree * Enemy::s_qPtr = nullptr;
sf::Vector2f Enemy::s_mapSize = sf::Vector2f(0,0);
int Enemy::s_Remaining = 0;
int Enemy::s_Spawn_Count = 0;

Enemy::Enemy() : Character()
{
	p_messageID = 1;
	m_shootTimer = Timer(true);
	m_enemyType = (EnemyType)(rand() % 2);

	//m_enemyType = Agressive;

	std::vector<StateMachine<Enemy>::State> stateVec;

	m_playerPosLastFrame = sf::Vector2f(0, 0);

	switch (m_enemyType)
	{
	case Enemy::Random:
		_setupRandom(stateVec);
		break;
	case Enemy::Agressive:
		_setupAgressive(stateVec);
		break;
	case Enemy::Passive:
		SetColor(sf::Color::White);
		_setupPassive(stateVec);
		break;
	}

	m_stateMachine.SetStates(stateVec);
}

void Enemy::SetMapSize(float x, float y)
{
	s_mapSize.x = x;
	s_mapSize.y = y;
}

void Enemy::SetPlayer(Character * player)
{
	s_playerPtr = player;
}

void Enemy::SetPathfindingAndQuadTree(Pathfinding * pf, QuadTree * q)
{
	s_pfPtr = pf;
	s_qPtr = q;
}

Character * Enemy::GetPlayerPtr()
{
	return s_playerPtr;
}

void Enemy::Shoot(int inaccuracy)
{
	using namespace DirectX;
	if (m_canShoot)
	{
		m_shootTimer.Stop();
		m_shoootTime = 0.0;
		auto pPos = GetPosition();
		sf::Vector2f target = s_playerPtr->GetPosition() + s_playerPtr->GetSize() * 0.5f;
		const std::vector<sf::Vector2f> & pPath = s_playerPtr->GetPath();
		if (!pPath.empty() && rand() % 4 == 0)
		{
			int wantedIndex = 0;
			float d = _distToPlayer() / m_projectileSpeed + 0.5f;
			wantedIndex = std::min((int)d, (int)pPath.size() - 1);

			target = pPath[wantedIndex];
		}

		sf::Vector2f inAc((rand() % inaccuracy) * (rand() % 2 == 0 ? -1 : 1), (rand() % inaccuracy) * (rand() % 2 == 0 ? -1 : 1));
		XMVECTOR dir = XMVector2Normalize(XMVectorSubtract(XMLoadFloat2((XMFLOAT2*)&(target + inAc)), XMLoadFloat2((XMFLOAT2*)&pPos)));
		dir = XMVectorScale(dir, m_projectileSpeed);
		sf::Vector2f sfDir;
		DirectX::XMStoreFloat2(((XMFLOAT2*)&sfDir), dir);

		Projectile::Create(pPos + GetSize() * 0.5f, sfDir, m_bulletLifeTime, 12, this);
		m_canShoot = false;
	}

	if (!m_canShoot)
	{
		m_shoootTime += m_shootTimer.Stop();

		if (m_shoootTime > (1.0 / m_shotsPerSecond))
		{
			m_canShoot = true;
		}
	}
}

void Enemy::Update(double dt, bool enableAI)
{
	p_messageID = 1;
	m_prcHp = Character::GetCurrentHealth() / Character::GetMaxHealth();
	if (enableAI)
	{
		sf::Vector2f pPos = s_playerPtr->GetPosition();
		m_playerMoved = false;
		if (fabs(m_playerPosLastFrame.x - pPos.x) > 0.00001f || fabs(m_playerPosLastFrame.y - pPos.y) > 0.00001f)
			m_playerMoved = true;
		m_playerPosLastFrame = pPos;

		m_stateMachine.Update(dt, this);
	}

	sf::Vector2f mPos = GetPosition();



	Character::Update(dt);

	sf::Vector2f points[] = {
		GetPosition() + sf::Vector2f(2.0f, 2.0f),
		points[0] + sf::Vector2f(GetSize().x - 2.0f, 0.0f),
		points[0] + sf::Vector2f(0.0f, GetSize().y - 2.0f),
		points[0] + GetSize() - sf::Vector2f(2.0f, 2.0f)
	};

	sf::Vector2f cent = GetPosition() + GetSize() * 0.5f;

	std::vector<unsigned long long> avoid(1);
	avoid[0] = GetUniqueID();

	Entity * e = nullptr;

	for (int i = 0; i < 4 && e == nullptr; i++)
	{
		e = s_qPtr->DispatchRay(cent, points[i], &avoid);
	}

	if (e && e->GetMessageID() == 1)
	{
		p_messageID = 2;
		SetPosition(mPos);
	}

}

#pragma region RandomState
void Enemy::_setupRandom(std::vector<StateMachine<Enemy>::State>& stateVector)
{
	StateMachine<Enemy>::State state;
	state.Name = "Pure_Random";
	state.BehaviourLogic = &Enemy::_pureRandom;
	state.NexStateCondition = &Enemy::_nextStateRandom;

	stateVector.push_back(state);

	state.Name = "Close_Random";
	state.BehaviourLogic = &Enemy::_closeRandom;
	stateVector.push_back(state);

	state.Name = "Keep_Distance_Random";
	state.BehaviourLogic = &Enemy::_keepDistanceRandom;
	stateVector.push_back(state);

	m_stateVar.changeStateTimer.Start();
	m_stateVar.minDist = (float)(rand() % 5 + 10);
	m_stateVar.maxDist = (float)(rand() % 5 + 20);
	m_stateVar.waitForPath = (float)(rand() % 4 + 1);
	m_bulletLifeTime = 8;

	SetColor(sf::Color::Magenta);
}

std::string Enemy::_nextStateRandom(double dt)
{
	if (m_stateMachine.GetCurrentState() != "Pure_Random" && m_stateVar.changeStateTime > 5.0 && rand() % 25 == 0)
	{
		m_stateVar.changeStateTime = 0.0;
		return "Pure_Random";
	}
	else if (m_stateMachine.GetCurrentState() != "Close_Random" && m_stateVar.changeStateTime > 5.0 && rand() % 5 == 0 && m_prcHp > 0.5)
	{
		m_stateVar.changeStateTime = 0.0;
		return "Close_Random";
	}
	else if (m_stateMachine.GetCurrentState() != "Keep_Distance_Random" && m_stateVar.changeStateTime > 5.0 && rand() % 5 == 0 && m_prcHp <= 0.5)
	{
		m_stateVar.changeStateTime = 0.0;
		return "Keep_Distance_Random";
	}
	m_stateVar.changeStateTime += m_stateVar.changeStateTimer.Stop();

	return m_stateMachine.GetCurrentState();
}

void Enemy::_pureRandom(double dt)
{
	if (GetPath().empty())
	{
		sf::Vector2f from = GetPosition() + GetSize() * 0.5f;
		sf::Vector2f to = from;

		to.x += ((float)(rand() % 100) * GetSize().x) * ((rand() % 2 == 0) ? 1.0f : -1.0f);
		to.y += ((float)(rand() % 100) * GetSize().y) * ((rand() % 2 == 0) ? 1.0f : -1.0f);
		SetPath(s_pfPtr->FindPath(from, to));
	}

	if (rand() % 800 == 0)
	{
		std::vector<unsigned long long> avoid(2);
		avoid[0] = GetUniqueID();
		avoid[1] = s_playerPtr->GetUniqueID();
		Entity * e = s_qPtr->DispatchRay(GetPosition() + GetSize() * 0.5f, s_playerPtr->GetPosition() + s_playerPtr->GetSize() * 0.5f, &avoid);

		if (e == nullptr)
			Shoot();
	}

}

void Enemy::_closeRandom(double dt)
{
	float dist = m_stateVar.minDist * std::max(GetSize().x, GetSize().y);
	if (GetPath().empty() || (m_playerMoved && _distToPlayer() > dist && m_stateVar.pathTime > m_stateVar.waitForPath))
	{
		sf::Vector2f from = GetPosition() + GetSize() * 0.5f;
		sf::Vector2f to = s_playerPtr->GetPosition();

		to.x += ((float)(rand() % (int)(dist + 0.5f))) * ((rand() % 2 == 0) ? 1.0f : -1.0f);
		to.y += ((float)(rand() % (int)(dist + 0.5f))) * ((rand() % 2 == 0) ? 1.0f : -1.0f);

		auto path = s_pfPtr->FindPath(from, to);
		if (!path.empty())
		{
			SetPath(path);
			m_stateVar.pathTime = 0.0;
			m_stateVar.pathTimer.Start();
		}
	}

	m_stateVar.pathTime += m_stateVar.pathTimer.Stop();

	if (rand() % 1000 == 0)
	{
		std::vector<unsigned long long> avoid(2);
		avoid[0] = GetUniqueID();
		avoid[1] = s_playerPtr->GetUniqueID();
		Entity * e = s_qPtr->DispatchRay(GetPosition() + GetSize() * 0.5f, s_playerPtr->GetPosition() + s_playerPtr->GetSize() * 0.5f, &avoid);

		if (e == nullptr)
			Shoot();
	}
}

void Enemy::_keepDistanceRandom(double dt)
{
	float dist = m_stateVar.maxDist * std::max(GetSize().x, GetSize().y);
	if (GetPath().empty() || (_distToPlayer() < dist && m_stateVar.pathTime > m_stateVar.waitForPath * m_prcHp))
	{
		sf::Vector2f from = GetPosition() + GetSize() * 0.5f;
		sf::Vector2f to = from;
		sf::Vector2f dir = _dirFromPlayer();

		dir.x *= ((float)(rand() % (int)(dist + 0.5f)));
		dir.y *= ((float)(rand() % (int)(dist + 0.5f)));
		auto path = s_pfPtr->FindPath(from, to + dir);
		if (!path.empty())
		{
			SetPath(path);
			m_stateVar.pathTime = 0.0;
			m_stateVar.pathTimer.Start();
		}
	}
	else if (_distToPlayer() > dist && m_stateVar.pathTime > m_stateVar.waitForPath)
	{
		
		sf::Vector2f from = GetPosition() + GetSize() * 0.5f;
		sf::Vector2f to = from;
		sf::Vector2f dir = _crossDirPlayer(rand() % 2);
		dir.x *= ((float)(rand() % (int)(dist + 0.5f)));
		dir.y *= ((float)(rand() % (int)(dist + 0.5f)));
		auto path = s_pfPtr->FindPath(from, to + dir);
		if (!path.empty())
		{
			SetPath(path);
			m_stateVar.pathTime = 0.0;
			m_stateVar.pathTimer.Start();
		}
	}

	m_stateVar.pathTime += m_stateVar.pathTimer.Stop();


	if (rand() % 500 == 0)
	{
		std::vector<unsigned long long> avoid(2);
		avoid[0] = GetUniqueID();
		avoid[1] = s_playerPtr->GetUniqueID();
		Entity * e = s_qPtr->DispatchRay(GetPosition() + GetSize() * 0.5f, s_playerPtr->GetPosition() + s_playerPtr->GetSize() * 0.5f, &avoid);

		if (e == nullptr)
			Shoot();
	}
}

#pragma endregion

#pragma region AgressiveState
void Enemy::_setupAgressive(std::vector<StateMachine<Enemy>::State>& stateVector)
{
	StateMachine<Enemy>::State state;

	state.Name = "Rest_Agressive";
	state.BehaviourLogic = &Enemy::_restAgressive;
	state.NexStateCondition = &Enemy::_leaveRestAgressive;
	stateVector.push_back(state);

	state.Name = "Charge_Agressive";
	state.BehaviourLogic = &Enemy::_chargeAgressive;
	state.NexStateCondition = &Enemy::_nextStateAgressive;

	stateVector.push_back(state);

	state.Name = "Strafe_Agressive";
	state.BehaviourLogic = &Enemy::_strafeAgressive;
	stateVector.push_back(state);

	m_stateVar.changeStateTimer.Start();
	m_stateVar.minDist = (float)(rand() % 5 + 5);
	m_stateVar.maxDist = (float)(rand() % 5 + 15);
	m_stateVar.waitForPath = 2;
	m_shotsPerSecond = 5.0;
	m_projectileSpeed = 640.0f;
	m_bulletLifeTime = 2;
	SetColor(sf::Color::Red);
}

std::string Enemy::_nextStateAgressive(double dt)
{
	float dist = _distToPlayer();

	if (dist > m_stateVar.maxDist * std::max(GetSize().x, GetSize().y))
	{
		return "Charge_Agressive";
	}
	else if (dist < m_stateVar.minDist * std::max(GetSize().x, GetSize().y))
	{
		return "Strafe_Agressive";
	}
	
	return m_stateMachine.GetCurrentState();
}

std::string Enemy::_leaveRestAgressive(double dt)
{
	std::vector<unsigned long long> avoid(2);
	avoid[0] = GetUniqueID();
	avoid[1] = s_playerPtr->GetUniqueID();

	Entity * e = s_qPtr->DispatchRay(GetPosition() + GetSize() * 0.5f, s_playerPtr->GetPosition() + GetSize() * 0.5f, &avoid);

	float eRemaining = (float)s_Remaining / (float)s_Spawn_Count;

	if (e == nullptr && ((m_playerMoved && _distToPlayer() < std::max(s_mapSize.x, s_mapSize.y) * 0.3)) || m_prcHp < 1.0 || eRemaining < 0.1f || s_Remaining <= 2)
	{
		m_playerMoved = true;
		m_stateVar.pathTime = 9999;
		return "Charge_Agressive";
	}

	return m_stateMachine.GetCurrentState();
}

void Enemy::_restAgressive(double dt)
{

}

void Enemy::_chargeAgressive(double dt)
{
	if (m_playerMoved && m_stateVar.pathTime > m_stateVar.waitForPath)
	{
		sf::Vector2f playerMoveTo = s_playerPtr->GetPosition();
		
		const std::vector<sf::Vector2f> & pPath = s_playerPtr->GetPath();

		if (!pPath.empty())
			playerMoveTo = pPath.back();

		auto path = s_pfPtr->FindPath(GetPosition() + GetSize() * 0.5f, playerMoveTo + s_playerPtr->GetSize() * 0.5f);
		if (!path.empty())
		{
			SetPath(path);
			m_stateVar.pathTime = 0.0;
			m_stateVar.pathTimer.Start();
		}
	}

	m_stateVar.pathTime += m_stateVar.pathTimer.Stop();

	if (true)
	{
		std::vector<unsigned long long> avoid(2);
		avoid[0] = GetUniqueID();
		avoid[1] = s_playerPtr->GetUniqueID();
		Entity * e = s_qPtr->DispatchRay(GetPosition() + GetSize() * 0.5f, s_playerPtr->GetPosition() + s_playerPtr->GetSize() * 0.5f, &avoid);

		if (e == nullptr)
			Shoot(128);
	}

}

void Enemy::_strafeAgressive(double dt)
{
	if (m_stateVar.pathTime > m_stateVar.waitForPath * 0.5f)
	{
		sf::Vector2f from = GetPosition() + GetSize() * 0.5f;
		sf::Vector2f dir = _crossDirPlayer(rand() % 2);
		dir.x *= 5.0f * GetSize().x;
		dir.y *= 5.0f * GetSize().y;
		sf::Vector2f to = from + dir;

		auto path = s_pfPtr->FindPath(from, to);
		if (!path.empty())
		{
			SetPath(path);
			m_stateVar.pathTime = 0.0;
			m_stateVar.pathTimer.Start();
		}
	}

	m_stateVar.pathTime += m_stateVar.pathTimer.Stop();

	if (rand() % 500)
	{
		std::vector<unsigned long long> avoid(2);
		avoid[0] = GetUniqueID();
		avoid[1] = s_playerPtr->GetUniqueID();
		Entity * e = s_qPtr->DispatchRay(GetPosition() + GetSize() * 0.5f, s_playerPtr->GetPosition() + s_playerPtr->GetSize() * 0.5f, &avoid);

		if (e == nullptr)
			Shoot();
	}

}
#pragma endregion

void Enemy::_setupPassive(std::vector<StateMachine<Enemy>::State>& stateVector)
{
}

std::string Enemy::_nextStatePassive(double dt)
{
	return std::string();
}

float Enemy::_distToPlayer()
{
	return DirectX::XMVectorGetX(
		DirectX::XMVector2Length(
			DirectX::XMVectorSubtract(
				XMLoadFloat2((DirectX::XMFLOAT2*)&s_playerPtr->GetPosition()),
				XMLoadFloat2((DirectX::XMFLOAT2*)&GetPosition()))));
}

sf::Vector2f Enemy::_dirFromPlayer()
{
	sf::Vector2f dir;
	DirectX::XMStoreFloat2((DirectX::XMFLOAT2*)&dir,
		DirectX::XMVector2Normalize(
			DirectX::XMVectorSubtract(
				XMLoadFloat2((DirectX::XMFLOAT2*)&GetPosition()),
				XMLoadFloat2((DirectX::XMFLOAT2*)&s_playerPtr->GetPosition())
			)
		)
	);
	return dir;
}

sf::Vector2f Enemy::_crossDirPlayer(int leftOrRight)
{
	sf::Vector2f dir = _dirFromPlayer();
	DirectX::XMFLOAT3 XZplane;
	XZplane.x = dir.x;
	XZplane.y = 0.0f;
	XZplane.z = dir.y;

	DirectX::XMFLOAT3 YAxis(0, 1, 0);
	DirectX::XMVECTOR vecDir;

	if (leftOrRight == 0)
		vecDir = DirectX::XMVector3Cross(DirectX::XMLoadFloat3(&XZplane), DirectX::XMLoadFloat3(&YAxis));
	else
		vecDir = DirectX::XMVector3Cross(DirectX::XMLoadFloat3(&YAxis), DirectX::XMLoadFloat3(&XZplane));

	DirectX::XMStoreFloat3(&XZplane, DirectX::XMVector3Normalize(vecDir));
	dir.x = XZplane.x;
	dir.y = XZplane.z;
	return dir;
}
