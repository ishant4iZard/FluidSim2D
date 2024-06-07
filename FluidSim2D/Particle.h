#pragma once
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

#include <vector>

class particle 
{
public:
	float mass = 1.0f;
	float radius = 1.0f;

	sf::Vector2f Position = sf::Vector2f();
	sf::Vector2f PredictedPosition = sf::Vector2f();
	sf::Vector2f Velocity = sf::Vector2f();
	sf::Vector2f Force = sf::Vector2f();
	sf::Vector2f Acceleration = sf::Vector2f();
	sf::Vector2f PressureAcceleration = sf::Vector2f();

	float density = 0;
	float pressure = 0;

	sf::Vector2f GridPos = sf::Vector2f();

	int Gridhash;

	particle() {};
	
};
