#pragma once
#include "SFML/Graphics.hpp"
#include "SFML/Window.hpp"

#include "Particle.h"
#include "ParticlePhysics.h"
#include "Timer.h"

class SimWindow 
{
private:
	sf::RenderWindow window;

	SPH* water;
	int numParticles;

	GameTimer timer;

	double dTOffset = 0;
	const int   idealHZ = 240;
	const float idealDT = 1.0f / idealHZ;

	int realHZ = idealHZ;
	float realDT = idealDT;

public:
	SimWindow(int width, int height, const char* name);
	~SimWindow();

	bool Update(double dt);

	GameTimer getTimer() {
		return timer;
	}
};