#pragma once
#include "Particles.h"
#include <thread>
#include <map>
#include <vector>

#define ull unsigned long long

class ParticleSystem
{
private:
	ParticleSystem();
	~ParticleSystem();

public:
	static ParticleSystem * GetInstance();

	void Begin(double dt);

	ull SubmitWork(const Particles & p);
	Particles * At(ull ID);
	void AbortWork(ull ID);

	void End();


	void Draw(sf::RenderWindow * wnd);

private:
	std::vector<ull> m_workAbortion;
	std::map<ull, Particles> m_queue;
	std::map<ull, Particles> m_work;

	ull			m_IDCounter = 0;

	bool		m_isRunning = true;
	bool		m_hasWork = false;
	double		m_deltaTime = 0.0;

	std::thread m_workingThread;

private:
	void _work();

};