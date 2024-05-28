#pragma once
#include "Particle.h"
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"
#include "HelperFunctions.h"
#include "ThreadPool.h"
#include <mutex>

#define PI 3.14159f

struct boundingArea {
	int left = 20;
	int right = 1240;
	int bottom = 700;
	int top = 20;
};

class SPH 
{
private:
	//particle* water;

#pragma region particles
	sf::CircleShape shape;
	int numParticles;
	float particleRadius;
	float smoothingRadius;
	float particleSpacing;
	float mass = 1.0f;

	particle* particles;

	float avgDensity;

	float dampingRate = 0.98f;

#pragma endregion

	int HorGrids;
	int VerGrids;

	boundingArea fence;

	bool gravityEnabled = 1;
	sf::Vector2f gravity = sf::Vector2f(0, 9.8f);

	float targetDensity = 0.02f;
	int pressureMultiplier = 100;

	std::vector<std::vector<std::unique_ptr<Grid>>> gridsys;

	std::mutex** gridMutexes;

	bool useOpenMp = 0;

#pragma region HelperFunctions
	float smoothingKernel(float inradius, float dst) {
		if (dst >= inradius) return 0;
		float volume = PI * pow(inradius, 4) / 6;
		return pow((inradius-dst),2)/ volume;
	}

	float smoothingKernerDerivative(float inradius, float dst) {
		if (dst >= inradius)return 0;
		float scale = 12 / (PI * pow(inradius, 4));
		return (dst- inradius) * scale;
	}

	float ConvertDensityToPressure(float density) {
		float deltaDensity = density - targetDensity;
		float m_pressure = deltaDensity * pressureMultiplier;
		return m_pressure;
	}

	float vectorMagnitude(sf::Vector2f vector2) {
		float r = sqrtf((vector2.x * vector2.x) + (vector2.y * vector2.y));
		return r;
	}

	sf::Vector2f GetRandomDir() {
		float x = rand() % 100;
		float y = sqrt(1-(x/100.0f)*(x/100.0f));

		sf::Vector2f a(x/100.0f, y);
		return a ;
	}

#pragma endregion

#pragma region PhysicsTimer
	double dTOffset = 0;
	const int   idealHZ = 60;
	const float idealDT = 1.0f / idealHZ;

	int realHZ = idealHZ;
	float realDT = idealDT;

#pragma endregion


	ThreadPool* threadpoolptr;

	void randomPositionStart(float screenWidth, float screeenHeight);
	void GridStart(float screenWidth, float screeenHeight);

	void PhysicsUpdate(float dt);
	void updateParticle(float dt);

	//void UpdateDensityandPressure();
	//void UpdatePressureAcceleration();
	
	void UpdateDensityandPressureGrid();
	void UpdatePressureAccelerationGrid();
	
	//void CreateGrids();
	void SetParticlesInGrids();
	void ClearGrids();

	std::vector<sf::Vector2f> findOffsetGrids(sf::Vector2f gridPos);

public:

	SPH(int inNumParticles, float screenWidth, float screeenHeight);
	~SPH();

	void Update(float dt);
	void Draw(sf::RenderWindow& window);
	
	//float calcDensity(int particleIndex);
	//sf::Vector2f calcPressureForce(int particleIndex);
	
	float calcDensityGrid(int particleIndex, sf::Vector2f gridPos);
	sf::Vector2f calcPressureForceGrid(int particleIndex, sf::Vector2f gridPos);

};
