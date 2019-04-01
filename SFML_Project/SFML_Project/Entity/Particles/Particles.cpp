#include "Particles.h"
#include "../../Camera/Camera.h"
Particles::Particles(unsigned int count, bool Loop)
{
	m_loop = Loop;

	m_particles = std::vector<Particle>(count);
	m_vertices = sf::VertexArray(sf::Points, count);

	m_color[0] = sf::Color::White;
	m_color[1] = sf::Color::White;
	m_color[1].a = 0;
	
	m_lifeTime = 2.0;
}

void Particles::SetColor(sf::Uint8 r, sf::Uint8 g, sf::Uint8 b, sf::Uint8 a)
{
	m_color[0] = sf::Color(r, g, b, a);
	m_color[1] = sf::Color(r, g, b, a);
}

void Particles::SetColor(const sf::Color & color)
{
	m_color[0] = color;
	m_color[1] = color;
}

void Particles::SetColor(const sf::Color color[2])
{
	m_color[0] = color[0];
	m_color[1] = color[1];
}

void Particles::SetPosition(const sf::Vector2f & position)
{
	m_worldPos = position;
}

void Particles::SetPosition(float x, float y)
{
	m_worldPos.x = x;
	m_worldPos.y = y;
}

void Particles::SetLoop(bool loop)
{
	m_loop = loop;
}

void Particles::Translate(const sf::Vector2f & translation)
{
	m_worldPos = m_worldPos + translation;
}

void Particles::Translate(float x, float y)
{
	m_worldPos.x += x;
	m_worldPos.y += y;
}

bool Particles::IsDone() const
{
	return m_isDone;
}

const sf::Vector2f & Particles::GetPosition() const
{
	return m_worldPos;
}

void Particles::Update(double dt)
{
	m_totalTime += dt;
	m_isDone = !m_loop;
	for (std::size_t i = 0; i < m_particles.size(); ++i)
	{
		Particle & p = m_particles[i];
		
		p.LifeTime -= dt;

		if (p.LifeTime <= 0.0 && (m_loop || m_firstRun))
			_resetParticle(i);

		else if (p.LifeTime > 0.0)
		{
			m_isDone = false;
		}

		p.Offset += p.Velocity * (float)dt;

		double ratio = p.LifeTime / m_lifeTime;
		p.Color = _Lerp(m_color[0], m_color[1], ratio);
	}
	m_firstRun = false;
}

void Particles::Draw(sf::RenderWindow * wnd)
{
	if (wnd)
	{
		sf::Vector2u ws = wnd->getSize();
		sf::FloatRect screenBox;
		screenBox.left = 0.0f;
		screenBox.top = 0.0f;
		screenBox.height = (float)ws.y;
		screenBox.width = (float)ws.x;

		_calcScreenPosition(screenBox);

		for (std::size_t i = 0; i < m_particles.size(); i++)
			_calcScreenPositionFor(i, screenBox);

		wnd->draw(m_vertices);
	}
}

void Particles::_resetParticle(unsigned int index)
{
	float angle = (std::rand() % 360) * 3.14f / 180.f;
	float speed = (std::rand() % 50) + 50.f;

	m_particles[index].Velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
	m_particles[index].LifeTime = (double)(rand() % 3000 + 500) / 1000.0;
	m_particles[index].Offset = sf::Vector2f(0, 0);
	m_particles[index].StartPos = m_worldPos;
	m_isDone = false;
}

void Particles::_calcScreenPosition(const sf::FloatRect & screenBox)
{
	Camera * cam = Camera::GetActiveCamera();

	if (cam)
	{
		sf::Vector3f camPos = cam->GetPosition();
		m_screenPos = m_worldPos - sf::Vector2f(camPos.x, camPos.y);
		m_screenPos *= (1.0f / camPos.z);
		m_screenPos.x += screenBox.width * 0.5f;
		m_screenPos.y += screenBox.height * 0.5f;
	}
}

void Particles::_calcScreenPositionFor(unsigned int index, const sf::FloatRect & screenBox)
{
	Camera * cam = Camera::GetActiveCamera();

	if (cam)
	{
		sf::Vector3f camPos = cam->GetPosition();
		Particle & p = m_particles[index];

		sf::Vector2f screenPos = (p.Offset + p.StartPos) - sf::Vector2f(camPos.x, camPos.y);
		screenPos *= (1.0f / camPos.z);
		screenPos.x += screenBox.width * 0.5f;
		screenPos.y += screenBox.height * 0.5f;
		m_vertices[index].color = sf::Color::Red;
		m_vertices[index].position = screenPos;
	}
}

sf::Color Particles::_Lerp(const sf::Color & a, const sf::Color & b, double t)
{
	sf::Color retCol;
	retCol.r = a.r + t * (b.r - a.r);
	retCol.g = a.g + t * (b.g - a.g);
	retCol.b = a.b + t * (b.b - a.b);
	retCol.a = a.a + t * (b.a - a.a);

	return retCol;
}
