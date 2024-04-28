#include "ParticlePhysics.h"
#include <iostream>
#include "SFML/Window.hpp"

SPH::SPH(int inNumParticles, float screenWidth, float screeenHeight)
{
    numParticles = inNumParticles;
    water = new particle[numParticles];
    particleRadius = water[0].getRadius();
    smoothingRadius = 10.0f;
    particleSpacing = 1.0f;
    int particlesPerRow = (int)sqrt(numParticles);
    int particlesPerCol = (numParticles - 1) / particlesPerRow + 1;
    float spacing = particleRadius * 2 + particleSpacing;
    for (int i = 0; i < numParticles; i++) {
        float x = (i % particlesPerRow - (particlesPerRow / 2.0f) + 5.0f) * spacing + screenWidth / 2;
        float y = (i / particlesPerRow - (particlesPerCol / 2.0f) + 5.0f) * spacing + screeenHeight / 2;
        water[i].setPosition(sf::Vector2f(x, y));
    }
}

void::SPH::Update(float dt) {
    PhysicsUpdate(dt);
}

void SPH::Draw(sf::RenderWindow& window)
{
    for (int i = 0; i < numParticles; i++) {
        window.draw(water[i].drawable());
    }
}

void SPH::PhysicsUpdate(float dt)
{
    dTOffset += dt;

    int iteratorCount = 0;
    while (dTOffset > realDT) {
        for (int i = 0; i < numParticles; i++) {
            water[i].Update(realDT);
        }
        dTOffset -= realDT;
        iteratorCount++;
    }
    if (iteratorCount > 0)
        clearForces();

    if (dt > realDT) {
        realHZ /= 2;
        realDT *= 2;
        std::cout << "Dropping iteration count due to long physics time...(now " << realHZ << ")\n";
    }
    else if (dt * 2 < realDT) { //we have plenty of room to increase iteration count!
        int temp = realHZ;
        realHZ *= 2;
        realDT /= 2;

        if (realHZ > idealHZ) {
            realHZ = idealHZ;
            realDT = idealDT;
        }
        if (temp != realHZ) {
            std::cout << "Raising iteration count due to short physics time...(now " << realHZ << ")\n";
        }
    }
}

float vectorMagnitude(sf::Vector2f vector2) {
    float r = sqrt((vector2.x * vector2.x) + (vector2.y * vector2.y));
    return r;
}

float SPH::calcDensity(sf::Vector2f samplePosition)
{
    float density = 0;
    const float mass = 1.0f;

    for (int i = 0; i < numParticles; i++) {
        float dst = vectorMagnitude(water[i].getPosition() - samplePosition);
        float influence = smoothingKernel(smoothingRadius, dst);
        density += mass * influence;
    }

    return density;
}


float SPH::calcProperty(sf::Vector2f samplePosition)
{
    float property = 0;
    const float mass = 1.0f;

    for (int i = 0; i < numParticles; i++) {
        float dst = vectorMagnitude(water[i].getPosition() - samplePosition);
        float influence = smoothingKernel(smoothingRadius, dst);
        float density = water[i].density;
        property += water[i].particleProperty * mass * influence / density;
    }

    return property;
}

sf::Vector2f SPH::calcPropertyGradient(sf::Vector2f samplePosition)
{
    sf::Vector2f propertyGradient = sf::Vector2f(0,0);

    for (int i = 0; i < numParticles; i++) {
        float dst = vectorMagnitude(water[i].getPosition() - samplePosition);
        sf::Vector2f dir = -(water[i].getPosition() - samplePosition) / dst;
        float slope = smoothingKernerDerivative(smoothingRadius, dst);
        float density = water[i].density;
        propertyGradient += dir * water[i].particleProperty * slope / density;
    }

    return propertyGradient;
}

void SPH::UpdateDensity() {
    for (int i = 0; i < numParticles; i++) {
        water[i].density = calcDensity(water[i].getPosition());
    }
}

void SPH::clearForces()
{
    for (int i = 0; i < numParticles; i++) {
        water[i].ClearForces();
    }
}
