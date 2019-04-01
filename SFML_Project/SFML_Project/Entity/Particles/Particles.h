#pragma once
#include <SFML/Graphics.hpp>


class Particles
{
public:
	Particles(unsigned int count = 1000, bool Loop = true);

	void SetColor(sf::Uint8 r, sf::Uint8 g, sf::Uint8 b, sf::Uint8 a = 255.0f);
	void SetColor(const sf::Color & color);
	void SetColor(const sf::Color color[2]);

	void SetPosition(const sf::Vector2f & position);
	void SetPosition(float x, float y);

	void SetLoop(bool loop);

	void Translate(const sf::Vector2f & translation);
	void Translate(float x, float y);

	bool IsDone() const;

	const sf::Vector2f & GetPosition() const;

	void Update(double dt);

	void Draw(sf::RenderWindow * wnd);

private:
	struct Particle
	{
		sf::Vector2f	StartPos;
		sf::Vector2f	Offset;
		sf::Vector2f	Velocity;
		sf::Color		Color;
		double			LifeTime = 0.0;
	};
	
	bool m_insideScreen = false;
	bool m_loop = false;
	bool m_isDone = false;
	bool m_firstRun = true;
	sf::FloatRect m_hitBox;

	std::vector<Particle>	m_particles;
	sf::VertexArray			m_vertices;
	double					m_lifeTime = 0.0;
	double					m_totalTime = 0.0;
	sf::Vector2f			m_worldPos;
	sf::Vector2f			m_screenPos;
	sf::Color				m_color[2];


private:
	void _resetParticle(unsigned int index);
	void _calcScreenPosition(const sf::FloatRect & screenBox);
	void _calcScreenPositionFor(unsigned int index, const sf::FloatRect & screenBox);

	sf::Color	_Lerp(const sf::Color & a, const sf::Color & b, double t);
};