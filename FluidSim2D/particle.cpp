#include "Particle.h"
#include <iostream>

particle::particle()
{
    mass = 1.0f;
    radius = 1.0f;
    position = sf::Vector2f();
    velocity = sf::Vector2f();
    acceleration = sf::Vector2f();
    force = sf::Vector2f();
    gravity = sf::Vector2f(0, 100.0f);//10 pixels = 1m

    shape.setRadius(radius);
    shape.setFillColor(sf::Color::Blue);
    shape.setPosition(sf::Vector2f(position.x - radius, position.y - radius));
    gravityEnabled = false;
    dampingRate = 0.8f;
}

void particle::Update(float dt)
{
    if (position.y >= fence.bottom - radius && velocity.y > 0) {
        velocity.y = -velocity.y * dampingRate;
    }
    if (position.y <= fence.top + radius && velocity.y < 0) {
        velocity.y = -velocity.y * dampingRate;
    }
    if (position.x >= fence.right - radius && velocity.x > 0) {
        velocity.x = -velocity.x * dampingRate;
    }
    if (position.x <= fence.left + radius && velocity.y < 0) {
        velocity.x = -velocity.x * dampingRate;
    }

    acceleration = force / mass;
    if (gravityEnabled)
        acceleration += gravity;
    velocity += acceleration * dt;
    setPosition(position + velocity * dt);
    //std::cout << velocity.x << " " << velocity.y << "\n";
}

void particle::addForce(sf::Vector2f forceToBeAdded)
{
    force += forceToBeAdded;
}

sf::Vector2f particle::getVelocity()
{
    return velocity;
}

void particle::setPosition(sf::Vector2f newPosition)
{
    position = newPosition;
    shape.setPosition(sf::Vector2f(position.x - radius, position.y - radius));
}

sf::Vector2f particle::getPosition()
{
    return position;
}

sf::CircleShape particle::drawable()
{
    return shape;
}
