#pragma once
#include "SFML/Graphics.hpp"
#include "SFML/Window.hpp"

#include "Particle.h"
#include "Timer.h"

class SimWindow 
{
private:
	sf::RenderWindow window;

	particle* water;
	float particleRadius;
	float particleSpacing;
	int numParticles;

	GameTimer timer;

	double dTOffset = 0;
	const int   idealHZ = 240;
	const float idealDT = 1.0f / idealHZ;

	int realHZ = idealHZ;
	float realDT = idealDT;

public:
	SimWindow(int width, int height, const char* name,int numParticles =100);
	bool Update(double dt);

	void ClearForces();

	GameTimer getTimer() {
		return timer;
	}
};