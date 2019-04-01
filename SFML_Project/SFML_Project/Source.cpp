#include "Engine/Engine.h"
#include "Utility/Timer.h"
#include <iostream>
int main()
{

	double timeCollector = 0.0;
	int frameCounter = 0;

	//sf::RenderWindow window(sf::VideoMode(1280, 720), "SFML works!", sf::Style::Fullscreen);
	sf::RenderWindow window(sf::VideoMode(1280, 720), "SFML works!");

	Engine game(&window);

	Timer deltaTime;
	deltaTime.Start();
	while (window.isOpen())
	{
		//std::cout << "PROJECTILES DO NOT HIT PLAYER, PLEASE FIX!!!\n";
		game.ClearEvent();

		sf::Event event;
		while (window.pollEvent(event))
			game.HandleEvent(event);

		window.clear();

		double time = deltaTime.Stop();
		game.Update(time);

		game.Draw();

		window.display();


		timeCollector += time;

		if (frameCounter++ > 60)
		{
			timeCollector /= (double)frameCounter;
			timeCollector *= 1000.0;
			window.setTitle("FrameTime: " + std::to_string(timeCollector) + " ms");
			timeCollector = 0.0;
			frameCounter = 0;
		}

	}

	return 0;
}