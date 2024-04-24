#pragma once
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

class particle 
{
private:
	sf::CircleShape shape;
	sf::Vector2f velocity;
	sf::Vector2f acceleration;
	sf::Vector2f force;
	float mass;

public:
	particle();
	
	void update();

	void addForce(sf::Vector2f forceToBeAdded);
	void draw();
};