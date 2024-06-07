#pragma once
#include "Particle.h"
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include <execution>

#define PI 3.14159f

struct boundingArea {
	int left = 20;
	int right = 1260;
	int bottom = 700;
	int top = 20;
};

class SPH 
{
private:

#pragma region particles
	particle* particles;

	int numParticles; 
	float particleRadius;
	float smoothingRadius;
	float particleSpacing;
	float mass = 1.0f;
	float dampingRate = 0.98f;

#pragma endregion

	int HorGrids;
	int VerGrids;
	int* hashLookupTable;

	boundingArea fence;

	bool gravityEnabled = 1;
	sf::Vector2f gravity = sf::Vector2f(0, 9.80f);

	double targetDensity = 20.f;
	float pressureMultiplier = 0.001f;
	float viscosityMultiplier = 3.f;

	sf::Vector2f offsetsGrids[9] = { 
		sf::Vector2f(0,0),
		sf::Vector2f(1,0),
		sf::Vector2f(1,1),
		sf::Vector2f(0,1),
		sf::Vector2f(-1,1),
		sf::Vector2f(-1,0),
		sf::Vector2f(-1,-1),
		sf::Vector2f(0,-1),
		sf::Vector2f(1,-1)

	};

	sf::VertexArray points;

#pragma region HelperFunctionsAndConstants
	double SmoothingKernelMultiplier;
	double SmoothingKernelDerivativeMultiplier;

	double smoothingKernel(float inradius, float dst) {
		if (dst >= inradius) return 0;
		return pow(((inradius - dst) / 100.0f), 2) * SmoothingKernelMultiplier;
	}

	double smoothingKernerDerivative(float inradius, float dst) {
		if (dst >= inradius)return 0;
		return ((dst- inradius) /100.0f) * SmoothingKernelDerivativeMultiplier;
	}

	double ConvertDensityToPressure(double density) {
		double deltaDensity = density - targetDensity;
		double m_pressure = deltaDensity * pressureMultiplier;
		return m_pressure;
	}

	float vectorMagnitude(sf::Vector2f vector2) {
		float r = sqrtf((vector2.x * vector2.x) + (vector2.y * vector2.y));
		return r;
	}

	sf::Vector2f GetRandomDir() {
		float x = (rand() % 100) / 100.0f;
		float y = sqrt(1 - (x * x));

		sf::Vector2f a(x, y);
		return a ;
	}

	int cellHash(int x, int y) {
		return y * HorGrids + x;
	}

	void resetHashLookupTable() {
		std::fill(std::execution::par, hashLookupTable, hashLookupTable + HorGrids*VerGrids, INT_MAX);
	}
#pragma endregion


	void randomPositionStart(float screenWidth, float screeenHeight);
	void GridStart(float screenWidth, float screeenHeight);


	double calcDensityGrid(int particleIndex, sf::Vector2f gridPos);
	sf::Vector2f calcPressureForceGrid(int particleIndex, sf::Vector2f gridPos);

	
	void SetParticlesInGridsHashing();
	void UpdateDensityandPressureGrid();
	void UpdatePressureAccelerationGrid();
	void updateParticle(float dt);

public:

	SPH(int inNumParticles, float screenWidth, float screeenHeight);
	~SPH();

	void Update(float dt);
	void Draw(sf::RenderWindow& window);
};
