#include "Map.h"
#include "../Pathfinding/Pathfinding.h"
#include <DirectXMath.h>

Map::Map()
{
}

Map::~Map()
{
}

void Map::CreateRandom(const sf::Vector2u & minSize, const sf::Vector2u & maxSize, const sf::Vector2f & tileSize)
{
	m_obsticlesSize = tileSize;
	m_obsticles.clear();

	m_mapSize.x = rand() % (maxSize.x - minSize.x) + minSize.x;
	m_mapSize.y = rand() % (maxSize.y - minSize.y) + minSize.y;

	Entity ob;
	ob.SetColor(sf::Color::Black);
	ob.SetSize(tileSize);
	for (unsigned int x = 0; x < m_mapSize.x; x++)
	{
		ob.SetPosition((float)x * tileSize.x, 0.0f);
		m_obsticles.push_back(ob);
		ob.SetPosition((float)x * tileSize.x, (m_mapSize.y - 1) * tileSize.y);
		m_obsticles.push_back(ob);
	}
	for (unsigned int y = 1; y < m_mapSize.y; y++)
	{
		ob.SetPosition(0.0f, (float)y * tileSize.y);
		m_obsticles.push_back(ob);
		ob.SetPosition((m_mapSize.x - 1) * tileSize.x, (float)y * tileSize.y);
		m_obsticles.push_back(ob);
	}

	int numberOfPillars = rand() % ((m_mapSize.x * m_mapSize.y) / 100);

	for (int i = 0; i < numberOfPillars; i++)
	{
		int x = rand() % (m_mapSize.x - 2) + 2;
		int y = rand() % (m_mapSize.y - 2) + 2;

		_placeMountain(x, y);
	}
}

void Map::SetBlockedTilesFor(Pathfinding * pf)
{
	size_t oSize = m_obsticles.size();

	for (size_t i = 0; i < oSize; i++)
	{
		sf::Vector2f pos = m_obsticles[i].GetPosition();

		pos.x /= m_obsticlesSize.x;
		pos.y /= m_obsticlesSize.y;
		pf->Block((sf::Vector2i)pos);
	}

}

std::vector<Entity>* Map::GetObsticles()
{
	return &m_obsticles;
}

const sf::Vector2u & Map::GetMapSize() const
{
	return m_mapSize;
}

void Map::Draw(sf::RenderWindow * wnd)
{
	size_t mapSize = m_obsticles.size();
	for (size_t i = 0; i < mapSize; i++)
	{
		m_obsticles[i].Draw(wnd);
	}
}

void Map::_placeMountain(int xPos, int yPos)
{
	int type = rand() % 3;
	int offset = 0;
	sf::Color startColor = sf::Color::White;
	switch (type)
	{
	case 1:		// Medium
		offset = 3;
		startColor = sf::Color(150, 75, 0);
		break;
	case 2:		// Large
		offset = 5;
		break;
	default:	// Small
		offset = 1;
		startColor = sf::Color(128, 128, 0);
		break;
	}
	float radius = std::max(m_obsticlesSize.x, m_obsticlesSize.y) * offset;

	using namespace DirectX;
	
	XMFLOAT2 startPos((float)xPos * m_obsticlesSize.x, (float)yPos* m_obsticlesSize.y);
	startPos.x += m_obsticlesSize.x * 0.5f;
	startPos.y += m_obsticlesSize.y * 0.5f;

	XMVECTOR vStart = XMLoadFloat2(&startPos);

	Entity ob;
	ob.SetColor(sf::Color::Black);
	ob.SetSize(m_obsticlesSize);
	for (int y = yPos - offset; y <= yPos + offset; y++)
	{
		for (int x = xPos - offset; x <= xPos + offset; x++)
		{
			if (x > 0 && y > 0 && x < (int)m_mapSize.x - 1 && y < (int)m_mapSize.y - 1)
			{
				XMFLOAT2 currentPos((float)x * m_obsticlesSize.x, (float)y* m_obsticlesSize.y);
				XMFLOAT2 cPosWithOffset = currentPos;
				cPosWithOffset.x += m_obsticlesSize.x * 0.5f;
				cPosWithOffset.y += m_obsticlesSize.y * 0.5f;
				XMVECTOR vCPos = XMLoadFloat2(&cPosWithOffset);
				XMVECTOR vDir = XMVectorSubtract(vCPos, vStart);
				float length = XMVectorGetX(XMVector2Length(vDir));
				if (length <= radius)
				{
					sf::Color c = startColor;

					if (length > 0.001f)
					{
						float p2 = radius / (length * 2);
						c.r *= p2;
						c.g *= p2;
						c.b *= p2;
					}

					ob.SetColor(c);
					ob.SetPosition(currentPos.x, currentPos.y);
					m_obsticles.push_back(ob);
				}
			}
		}
	}
}
