#pragma once
#include "Particle.h"
#include "HelperFunctions.h"


class SPH 
{
private:
	particle* water;
	int numParticles;
	float particleRadius;
	float smoothingRadius;
	float particleSpacing;

	float smoothingKernel(float inradius, float dst) {
		float volume = 3.141 * pow(inradius, 8) / 4;
		float value = std::max(0.0f, ((inradius * inradius) - (dst * dst)));
		return value * value * value / volume;
	}

	float smoothingKernerDerivative(float inradius, float dst) {
		if (dst > inradius)return 0;
		float f = inradius * inradius - dst * dst;
		float scale = -24 / (3.141 * pow(inradius, 8));
		return scale * dst * f * f;
	}

	double dTOffset = 0;
	const int   idealHZ = 60;
	const float idealDT = 1.0f / idealHZ;

	int realHZ = idealHZ;
	float realDT = idealDT;

	void PhysicsUpdate(float dt);
public:

	SPH(int inNumParticles, float screenWidth, float screeenHeight);

	void Update(float dt);
	void Draw(sf::RenderWindow& window);
	
	float calcDensity(sf::Vector2f samplePosition);
	float calcProperty(sf::Vector2f samplePosition);
	sf::Vector2f calcPropertyGradient(sf::Vector2f samplePosition);



	void UpdateDensity();
	void clearForces();
};
