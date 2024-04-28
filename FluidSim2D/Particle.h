#pragma once
#include "SFML/Window.hpp"
#include "SFML/Graphics.hpp"

struct boundingArea {
	int left = 20;
	int right = 1240;
	int bottom = 700;
	int top = 20;
};

class particle 
{
private:
	float mass;
	sf::CircleShape shape;
	float radius;

	sf::Vector2f position;
	sf::Vector2f velocity;
	sf::Vector2f acceleration;
	sf::Vector2f force;
	sf::Vector2f gravity;


	boundingArea fence;

	float dampingRate;
	bool gravityEnabled;


public:
	float particleProperty;
	float density;

	particle();
	
	void Update(float dt);
	sf::CircleShape drawable();

	void setPosition(sf::Vector2f newPosition);
	sf::Vector2f getPosition();

	sf::Vector2f getVelocity();

	void addForce(sf::Vector2f forceToBeAdded);

	void useGravity(bool is) {
		gravityEnabled = is;
	}

	void ClearForces() {
		force = sf::Vector2f();
	}

	float getRadius() {
		return radius;
	}
	void setRadius(float newRadius) {
		radius = newRadius;
		shape.setRadius(radius);
	}
};