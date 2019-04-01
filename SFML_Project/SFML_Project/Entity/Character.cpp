#include "Character.h"
#include "../Pathfinding/Tile.h"
#include <DirectXMath.h>
Character::Character() : Entity()
{
	
}

Character::~Character()
{

}
void Character::Update(double dt)
{
	if (!m_path.empty())
	{
		sf::Vector2f targetPosition = m_path.front();
		sf::Vector2f myPos = GetPosition() + GetSize() * 0.5f;
		
		sf::Vector2f direction = targetPosition - myPos;
		DirectX::XMFLOAT2 xmDir;
		DirectX::XMVECTOR xmVDir = DirectX::XMVectorSet(direction.x, direction.y, 0.0f, 0.0f);
		DirectX::XMStoreFloat2(&xmDir, DirectX::XMVector2Normalize(xmVDir));
		sf::Vector2f moveDir;
		moveDir.x = xmDir.x * (float)dt * m_speed;
		moveDir.y = xmDir.y * (float)dt * m_speed;

		if (!m_path.empty())
		{
			if (fabs(direction.x) < (float)dt * m_speed && fabs(direction.y) < (float)dt * m_speed)
			{
				m_path.erase(m_path.begin());
				m_pathDraw.erase(m_pathDraw.begin());
			}	
		}

		Translate(moveDir);

		if (p_pTexture)
		{
			/*p_time += dt;
			if (p_time > 1.0f)
			{
				p_time = 0.0;
				p_textureFrame.x++;
				if (p_textureFrame.x == p_pTexture->GetNrOfFrames().x)
				{
					p_textureFrame.x = 0;
					p_textureFrame.y++;

					if (p_textureFrame.y == p_pTexture->GetNrOfFrames().y)
					{
						p_textureFrame.y = 0;
					}
				}

				sf::IntRect r = p_pTexture->GetArea();
				r.left = p_textureFrame.x * r.width;
				r.top = p_textureFrame.y * r.height;
				p_spr.setTextureRect(r);
			}*/

			sf::IntRect r = p_pTexture->GetArea();
			if (xmDir.x < -0.33 && xmDir.y < -0.33f)	// UPLEFT
				p_textureFrame.y = 7;
			else if (xmDir.x > 0.33 && xmDir.y < -0.33f)	// UPRIGHT
				p_textureFrame.y = 1;
			else if (xmDir.x < -0.33 && xmDir.y >= 0.33f)	// DOWNLEFT
				p_textureFrame.y = 5;
			else if (xmDir.x > 0.33 && xmDir.y >= 0.33f)	// DOWNRIGHT
				p_textureFrame.y = 3;
			else if (xmDir.y < -0.33f)		// UP
				p_textureFrame.y = 0;
			else if (xmDir.y > 0.33f)	// DOWN
				p_textureFrame.y = 4;
			else if (xmDir.x > 0.33)	// RIGHT
				p_textureFrame.y = 2;
			else if (xmDir.x < -0.33)	// LEFT
				p_textureFrame.y = 6;

			r.left = p_textureFrame.x * r.width;
			r.top = p_textureFrame.y * r.height;
			p_spr.setTextureRect(r);
		}
	}
	
	m_healthbar.FollowThis = this;

	m_healthbar.Procent = m_currentHealth / m_maxHealth;

	m_healthbar.Update();

}

void Character::SetPath(const std::vector<sf::Vector2f> & path)
{
	m_path = path;
	size_t pSize = m_path.size();
	m_pathDraw = std::vector<Line>(pSize);
	if (!m_path.empty())
	{
		for (int i = 0; i < pSize - 1; i++)
		{
			Line l;
			l.SetColor(sf::Color::Black);
			l.SetLine(m_path[i], m_path[i + 1]);
			m_pathDraw[i] = l;
		}
	}
}

void Character::SetMaxHealth(float maxHealth)
{
	m_maxHealth = std::max(maxHealth, 1.0f);
}

void Character::SetCurrentHealth(float currentHealth)
{
	m_currentHealth = std::min(currentHealth, m_maxHealth);
	m_currentHealth = std::max(currentHealth, 0.0f);
}

void Character::AdjustCurrentHealth(float adjustMentHealth)
{
	m_currentHealth += adjustMentHealth;
	m_currentHealth = std::min(m_currentHealth, m_maxHealth);
	m_currentHealth = std::max(m_currentHealth, 0.0f);
}

float Character::GetMaxHealth() const
{
	return m_maxHealth;
}

float Character::GetCurrentHealth() const
{
	return m_currentHealth;
}

bool Character::IsDead() const
{
	return m_currentHealth < 0.0001f;
}

const std::vector<sf::Vector2f>& Character::GetPath() const
{
	return m_path;
}

void Character::SetSpeed(float speed)
{
	m_speed = speed;
}

void Character::Draw(sf::RenderWindow * wnd)
{
	m_healthbar.Draw(wnd);

	for (auto & d : m_pathDraw)
	{
		d.Draw(wnd);
	}
	Entity::Draw(wnd);
}
