#include "ParticlePhysics.h"
#include "SFML/Window.hpp"
#include <algorithm>
#include <iostream>
#include <omp.h>

SPH::SPH(int inNumParticles, float screenWidth, float screenHeight)
{
    numParticles = inNumParticles;
    particles = new particle[numParticles];

    particleRadius = 0.5f;

    smoothingRadius = 10.0f;
    particleSpacing = 0.5f;

    SmoothingKernelMultiplier = 5 * (6 / (PI * pow(smoothingRadius / 100, 4)));
    SmoothingKernelDerivativeMultiplier = 5 * (12 / (PI * pow(smoothingRadius / 100, 4)));

    HorGrids = (screenWidth / smoothingRadius) + 1;
    VerGrids = (screenHeight / smoothingRadius) + 1;

    hashLookupTable = new int[HorGrids * VerGrids];
    resetHashLookupTable();

    GridStart(screenWidth, screenHeight);
    //randomPositionStart(screenWidth, screenHeight);

    points = sf::VertexArray(sf::Points, numParticles);

}

SPH::~SPH()
{
    delete hashLookupTable;
    delete[]particles;
}

void SPH::Update(float dt) {

    SetParticlesInGridsHashing();
    UpdateDensityandPressureGrid();
    UpdatePressureAccelerationGrid();
    updateParticle(dt);
    resetHashLookupTable();
}

void SPH::Draw(sf::RenderWindow& window)
{
#pragma omp parallel for
    for (int i = 0; i < numParticles; i++) {
        points[i].position = particles[i].Position;
        points[i].color = sf::Color::Blue;
    }

    window.draw(points);

}

void SPH::randomPositionStart(float screenWidth, float screeenHeight)
{
    for (int i = 0; i < numParticles; i++) {
        float x = rand() % (int)screenWidth;
        float y = rand() % (int)screeenHeight;
        particles[i].Position = sf::Vector2f(x, y);
    }
}

void SPH::GridStart(float screenWidth, float screeenHeight)
{
    sf::Vector2f offsetVec(0, 0);
    int particlesPerRow = (int)sqrt(numParticles);
    int particlesPerCol = (numParticles - 1) / particlesPerRow + 1;
    float spacing = particleRadius * 2 + particleSpacing;
    for (int i = 0; i < numParticles; i++) {
        float x = (i % particlesPerRow - (particlesPerRow / 2.0f) + 5.0f) * spacing + (rand() % (int)(spacing * 100) / 100.f) + screenWidth / 2 + offsetVec.x;
        float y = (i / particlesPerRow - (particlesPerCol / 2.0f) + 5.0f) * spacing + (rand() % (int)(spacing * 100) / 100.f) + screeenHeight / 2 + offsetVec.y;
        particles[i].Position = sf::Vector2f(x, y);
        particles[i].PredictedPosition = sf::Vector2f(x, y);
    }
}

void SPH::updateParticle(float dt)
{

#pragma omp parallel for
    for (int i = 0; i < numParticles; i++) {
        particles[i].Acceleration = sf::Vector2f();
        particles[i].Velocity += particles[i].PressureAcceleration * dt;
        if (gravityEnabled)
            particles[i].Acceleration += gravity;
        particles[i].Velocity += particles[i].Acceleration * dt;
        particles[i].Position += particles[i].Velocity * dt;
        if (particles[i].Position.y >= fence.bottom - particleRadius && particles[i].Velocity.y > 0) {
            particles[i].Position.y = fence.bottom - (0.0001f + particleRadius);
            particles[i].Velocity.y = -particles[i].Velocity.y * dampingRate;
        }
        if (particles[i].Position.y <= fence.top + particleRadius && particles[i].Velocity.y < 0) {
            particles[i].Position.y = fence.top + (0.0001f + particleRadius);
            particles[i].Velocity.y = -particles[i].Velocity.y * dampingRate;
        }
        if (particles[i].Position.x >= fence.right - particleRadius && particles[i].Velocity.x > 0) {
            particles[i].Position.x = fence.right - (0.0001f + particleRadius);
            particles[i].Velocity.x = -particles[i].Velocity.x * dampingRate;
        }
        if (particles[i].Position.x <= fence.left + particleRadius && particles[i].Velocity.x < 0) {
            particles[i].Position.x = fence.left + (0.0001f + particleRadius);
            particles[i].Velocity.x = -particles[i].Velocity.x * dampingRate;
        }
        particles[i].PredictedPosition = particles[i].Position + particles[i].Velocity * (1 / 30.0f) + 0.5f * particles[i].Acceleration * (1 / 30.0f) * (1 / 30.0f);

        particles[i].PredictedPosition.x = std::clamp(particles[i].PredictedPosition.x, (float)fence.left, (float)fence.right);
        particles[i].PredictedPosition.y = std::clamp(particles[i].PredictedPosition.y, (float)fence.top, (float)fence.bottom);
    };

}

double SPH::calcDensityGrid(int particleIndex, sf::Vector2f gridPos)
{
    double density = 0;

    for (short Xoff = -1; Xoff < 2; Xoff++) {
        for (short Yoff = -1; Yoff < 2; Yoff++) {
            int key = cellHash(gridPos.x + Xoff, gridPos.y + Yoff);
            int startIndex = hashLookupTable[key];
            for (int i = startIndex; i < numParticles; i++) {
                if (key != particles[i].Gridhash) break;

                float dst = vectorMagnitude(particles[i].PredictedPosition - particles[particleIndex].PredictedPosition);
                double influence = smoothingKernel(dst);
                density += mass * influence;
            }
        }
    }

    return density;
}

sf::Vector2f SPH::calcPressureForceGrid(int particleIndex, sf::Vector2f gridPos)
{
    sf::Vector2f pressureForce = sf::Vector2f(0, 0);

    for (short Xoff = -1; Xoff < 2; Xoff++) {
        for (short Yoff = -1; Yoff < 2; Yoff++) {
            int key = cellHash(gridPos.x + Xoff, gridPos.y + Yoff);
            int startIndex = hashLookupTable[key];
            for (int i = startIndex; i < numParticles; i++) {
                if (key != particles[i].Gridhash) break;

                if (particleIndex == i) continue;

                sf::Vector2f offsetvec(particles[i].PredictedPosition - particles[particleIndex].PredictedPosition);

                float dst = vectorMagnitude(offsetvec);
                if (dst > smoothingRadius) continue;
                sf::Vector2f dir = dst == 0 ? GetRandomDir() : (offsetvec) / dst;
                double m_slope = smoothingKernerDerivative(dst);
                double m_density = particles[i].density;
                double sharedPressure = (particles[i].pressure + particles[particleIndex].pressure) / 2;
                pressureForce += dir * (float)(sharedPressure * m_slope * mass / m_density);

                //add Viscoscity
                sf::Vector2f velocityDiff = particles[i].Velocity - particles[particleIndex].Velocity;
                pressureForce += viscosityMultiplier * velocityDiff * (float)(-m_slope / (m_density * 100));
            }
        }
    }

    return pressureForce;
}

void SPH::UpdateDensityandPressureGrid()
{

#pragma omp parallel for
    for (int i = 0; i < numParticles; i++) {
        particles[i].density = calcDensityGrid(i, particles[i].GridPos);
        particles[i].pressure = ConvertDensityToPressure(particles[i].density);
    };

}

void SPH::UpdatePressureAccelerationGrid()
{
#pragma omp parallel for
    for (int i = 0; i < numParticles; i++) {
        particles[i].PressureAcceleration = calcPressureForceGrid(i, particles[i].GridPos);
    };

}

void SPH::SetParticlesInGridsHashing()
{
#pragma omp parallel for
    for (int i = 0; i < numParticles; i++) {
        int gridX = particles[i].PredictedPosition.x / smoothingRadius;
        int gridY = particles[i].PredictedPosition.y / smoothingRadius;
        particles[i].Gridhash = cellHash(gridX, gridY);
        particles[i].GridPos = sf::Vector2f(gridX, gridY);
    };

    //sort particles according to hash
    std::sort(std::execution::par, particles, particles + numParticles, [](const particle& a, const particle& b) {
        return a.Gridhash < b.Gridhash;
        });

    //create hash lookup for faster navigation
    hashLookupTable[particles[0].Gridhash] = 0;
    for (int i = 1; i < numParticles; i++) {
        if (particles[i].Gridhash != particles[i - 1].Gridhash)
        {
            hashLookupTable[particles[i].Gridhash] = i;
        }
    }
}