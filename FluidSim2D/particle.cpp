#include "Particle.h"
#include <iostream>

particle::particle()
{
    mass = 1.0f;
    radius = 1.0f;
    Position = sf::Vector2f();
    PredictedPosition = sf::Vector2f();
    Velocity = sf::Vector2f();
    Force = sf::Vector2f();
    Acceleration = sf::Vector2f();
    PressureAcceleration = sf::Vector2f();

    density = 0;
    pressure = 0;
}
