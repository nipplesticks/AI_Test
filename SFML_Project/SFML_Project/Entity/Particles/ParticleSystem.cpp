#include "ParticleSystem.h"

ParticleSystem::ParticleSystem()
{
	m_hasWork = false;
	m_workingThread = std::thread(&ParticleSystem::_work, this);
}

ParticleSystem::~ParticleSystem()
{
	m_isRunning = false;
	m_workingThread.join();
}

ParticleSystem * ParticleSystem::GetInstance()
{
	static ParticleSystem p;
	return &p;
}

void ParticleSystem::Begin(double dt)
{
	m_deltaTime = dt;

	std::vector<ull> terminate;

	for (auto & id : m_workAbortion)
		m_work[id].SetLoop(false);

	for (auto & p : m_work)
		if (p.second.IsDone())
			terminate.push_back(p.first);

	for (auto & ID : terminate)
		m_work.erase(ID);
	m_workAbortion.clear();

	for (auto & p : m_queue)
	{
		m_work.insert(std::make_pair(p.first, p.second));
	}

	m_queue.clear();

	if (m_workingThread.get_id() == std::thread::id())
	{
		m_workingThread = std::thread(&ParticleSystem::_work, this);
	}

	m_hasWork = true;
}

ull ParticleSystem::SubmitWork(const Particles & p)
{
	if (m_hasWork)
		m_queue.insert(std::make_pair(m_IDCounter, p));
	else
		m_work.insert(std::make_pair(m_IDCounter, p));

	return m_IDCounter++;
}

Particles * ParticleSystem::At(ull ID)
{
	return &m_work[ID];
}

void ParticleSystem::AbortWork(ull ID)
{
	m_workAbortion.push_back(ID);
}

#pragma optimize("", off)
void ParticleSystem::End()
{
	while (m_hasWork && m_workingThread.get_id() != std::thread::id());
}
#pragma optimize("", on)


void ParticleSystem::Draw(sf::RenderWindow * wnd)
{
	for (auto & p : m_work)
	{
		p.second.Draw(wnd);
	}
}


void ParticleSystem::_work()
{
	while (m_isRunning)
	{
		if (m_hasWork)
		{
			for (auto & p : m_work)
			{
				p.second.Update(m_deltaTime);
			}

			m_hasWork = false;
		}
	}
}
