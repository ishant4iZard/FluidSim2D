#pragma once
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

#include <vector>

class particle 
{
public:
	float mass;
	float radius;

	sf::Vector2f Position;
	sf::Vector2f PredictedPosition;
	sf::Vector2f Velocity;
	sf::Vector2f Force;
	sf::Vector2f Acceleration;
	sf::Vector2f PressureAcceleration;

	float density;
	float pressure;

	sf::Vector2f GridPos;

	int Gridhash;

	particle();
	
};

struct Grid {
	std::vector <int> gridParticles;

	std::vector<sf::Vector2f> offsetGrids;
};