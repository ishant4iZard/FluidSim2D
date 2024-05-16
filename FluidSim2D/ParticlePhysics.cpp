#include "ParticlePhysics.h"
#include <iostream>
#include "SFML/Window.hpp"

SPH::SPH(int inNumParticles, float screenWidth, float screenHeight)
{
    numParticles = inNumParticles;
    Position    = new sf::Vector2f[numParticles];
    Velocity    = new sf::Vector2f[numParticles];
    Force       = new sf::Vector2f[numParticles];
    Acceleration= new sf::Vector2f[numParticles];
    PressureAcceleration = new sf::Vector2f[numParticles];
    density     = new float[numParticles];
    pressure    = new float[numParticles];
    particleRadius = 2.0f;
    shape.setRadius(particleRadius);
    smoothingRadius = 50.0f;
    particleSpacing = 1.0f;
    threadpoolptr = new ThreadPool(3);

    GridStart(screenWidth, screenHeight);
    //randomPositionStart(screenWidth, screenHeight);

    UpdateDensity();
    UpdatePressure();
    UpdatePressureAcceleration();
}



SPH::~SPH()
{
    delete threadpoolptr;
}

void::SPH::Update(float dt) {
    PhysicsUpdate(dt);
}

void SPH::Draw(sf::RenderWindow& window)
{
    for (int i = 0; i < numParticles; i++) {
        shape.setPosition(Position[i]);
        if (pressure[i] > 0) {
            shape.setFillColor(sf::Color::Red);
        }
        else if(pressure[i] < 0){
            shape.setFillColor(sf::Color::Blue);
        }
        else{
            shape.setFillColor(sf::Color::White);
        }
        window.draw(shape);
    }
}

void SPH::randomPositionStart(float screenWidth, float screeenHeight)
{
    for (int i = 0; i < numParticles; i++) {
        int x = rand() % (int)screenWidth;
        float y = rand() % (int)screeenHeight;
        Position[i] = sf::Vector2f(x, y);
    }
}

void SPH::GridStart(float screenWidth, float screeenHeight)
{
    int particlesPerRow = (int)sqrt(numParticles);
    int particlesPerCol = (numParticles - 1) / particlesPerRow + 1;
    float spacing = particleRadius * 2 + particleSpacing;
    for (int i = 0; i < numParticles; i++) {
        float x = (i % particlesPerRow - (particlesPerRow / 2.0f) + 5.0f) * spacing + screenWidth / 2;
        float y = (i / particlesPerRow - (particlesPerCol / 2.0f) + 5.0f) * spacing + screeenHeight / 2;
        Position[i] = sf::Vector2f(x, y);
    }
}

void SPH::PhysicsUpdate(float dt)
{
    dTOffset += dt;

    int iteratorCount = 0;
    while (dTOffset > realDT) {
        threadpoolptr->enqueue([this]() {this->UpdateDensity(); });
        threadpoolptr->enqueue([this]() {this->UpdatePressure(); });
        threadpoolptr->enqueue([this]() {this->UpdatePressureAcceleration(); });
        threadpoolptr->enqueue([this]() {this->updateParticle(realDT); });

        //updateParticle(realDT);

        dTOffset -= realDT;
        iteratorCount++;
    }
    if (iteratorCount > 0)
        //clearForces();

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

void SPH::updateParticle(float dt)
{
    for (int i = 0; i < numParticles; i++) {

        Acceleration[i] = Force[i] / mass; 
        Velocity[i] += PressureAcceleration[i] * dt;
        if (gravityEnabled)
            Acceleration[i] += gravity;
        Velocity[i] += Acceleration[i] * dt;
        Position[i] += Velocity[i] * dt;
        if (Position[i].y >= fence.bottom - particleRadius && Velocity[i].y > 0) {
            Position[i].y = fence.bottom;
            Velocity[i].y = -Velocity[i].y * dampingRate;
        }
        if (Position[i].y <= fence.top + particleRadius && Velocity[i].y < 0) {
            Position[i].y = fence.top;
            Velocity[i].y = -Velocity[i].y * dampingRate;
        }
        if (Position[i].x >= fence.right - particleRadius && Velocity[i].x > 0) {
            Position[i].x = fence.right;
            Velocity[i].x = -Velocity[i].x * dampingRate;
        }
        if (Position[i].x <= fence.left + particleRadius && Velocity[i].x < 0) {
            Position[i].x = fence.left;
            Velocity[i].x = -Velocity[i].x * dampingRate;
        }

    }
}


float SPH::calcDensity(sf::Vector2f samplePosition)
{
    float density = 0;

    for (int i = 0; i < numParticles; i++) {
        float dst = vectorMagnitude(Position[i] - samplePosition);
        float influence = smoothingKernel(smoothingRadius, dst);
        density += mass * influence;
    }

    return density;
}

sf::Vector2f SPH::calcPressureForce(int particleIndex)
{
    sf::Vector2f pressureForce = sf::Vector2f(0,0);

    for (int i = 0; i < numParticles; i++) {
        if (particleIndex == i) continue;

        sf::Vector2f offset(Position[i] - Position[particleIndex]);

        float dst = vectorMagnitude(offset);
        sf::Vector2f dir = dst == 0 ? GetRandomDir() : (offset) / dst;
        float m_slope = smoothingKernerDerivative(smoothingRadius, dst);
        float m_density = density[i];
        float sharedPressure = (pressure[i] + pressure[particleIndex]) / 2;
        pressureForce += dir * sharedPressure * m_slope * mass / m_density;
    }

    return pressureForce;
}

void SPH::UpdateDensity() {
    avgDensity = 0;
    for (int i = 0; i < numParticles; i++) {
        density[i] = calcDensity(Position[i]);
        avgDensity += density[i];
    }
    avgDensity /= numParticles;
}


void SPH::UpdatePressure() {
    for (int i = 0; i < numParticles; i++) {
        pressure[i] = ConvertDensityToPressure(density[i]);
    }
}
void SPH::UpdatePressureAcceleration() {
    for (int i = 0; i < numParticles; i++) {
        PressureAcceleration[i] = calcPressureForce(i) / density[i];
    }
}

