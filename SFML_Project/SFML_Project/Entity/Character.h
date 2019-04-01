#pragma once
#include "Entity.h"
#include "Line.h"

class Character : public Entity
{
public:
	Character();
	~Character();

	void Update(double dt);
	void SetPath(const std::vector<sf::Vector2f> & path = std::vector<sf::Vector2f>());

	void SetMaxHealth(float maxHealth);
	void SetCurrentHealth(float currentHealth);
	void AdjustCurrentHealth(float adjustMentHealth);

	float GetMaxHealth() const;
	float GetCurrentHealth() const;
	bool IsDead() const;
	
	const std::vector<sf::Vector2f> & GetPath() const;
	void SetSpeed(float speed);

	void Draw(sf::RenderWindow * wnd) override;

private:
	std::vector<sf::Vector2f> m_path;
	std::vector<Line> m_pathDraw;
	
	struct Healthbar
	{
		Healthbar(Entity * followThis = nullptr)
		{
			FollowThis = followThis;
			Back.SetSize(64.0f, 10.0f);
			Back.SetColor(sf::Color::Red);
			Back.SetOutlineThickness(-1.0f);
			Back.SetOutlineColor(sf::Color::Black);
			Front = Back;
			Front.SetColor(sf::Color::Green);
			Procent = 1.0f;
		}

		void Update()
		{
			if (FollowThis)
			{
				Procent = std::min(Procent, 100.0f);
				Procent = std::max(Procent, 0.0f);

				float zoom = Camera::GetActiveCamera()->GetPosition().z;
				sf::Vector2f offset = FollowThis->GetSize();
				offset.x *= 0.5f;
				offset.y = 0.0f;
				Back.SetPosition(FollowThis->GetPosition() + offset);
				
				Back.SetSize(64.0f * zoom, 10.0f * zoom);
				Back.Translate(Back.GetSize().x * -0.5f, -Back.GetSize().y * 1.5f);
				Front.SetPosition(Back.GetPosition());
				Front.SetSize(Back.GetSize().x * Procent, Back.GetSize().y);
			}
		}

		void Draw(sf::RenderWindow * wnd)
		{
			Back.Draw(wnd);
			Front.Draw(wnd);
		}

		Entity Back;
		Entity Front;
		Entity * FollowThis;
		float Procent;
	}m_healthbar;


	float m_currentHealth = 100.0f;
	float m_maxHealth = 100.0f;

	float m_speed = 100.0f;
};