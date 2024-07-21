#include "ParticlePhysics.h"
#include "SFML/Window.hpp"
#include <algorithm>
#include <iostream>

SPH::SPH(int inNumParticles, float screenWidth, float screenHeight)
{
    numParticles = inNumParticles;
    particles = new particle[numParticles];

    particleRadius = 0.5f;

    smoothingRadius = 10.0f;
    particleSpacing = 0.5f;

    SmoothingKernelMultiplier = 5 * (6 / (PI * pow(smoothingRadius/100, 4)));
    SmoothingKernelDerivativeMultiplier = 5 * (12 / (PI * pow(smoothingRadius/100, 4)));

    HorGrids = (screenWidth / smoothingRadius) + 1;
    VerGrids = (screenHeight / smoothingRadius) + 1;

    hashLookupTable = new int [HorGrids * VerGrids];
    resetHashLookupTable();

    GridStart(screenWidth, screenHeight);
    //randomPositionStart(screenWidth, screenHeight);
    
    points = sf::VertexArray(sf::Points, numParticles);

}

SPH::~SPH()
{
    delete hashLookupTable;
    delete []particles;
}

void::SPH::Update(float dt) {

    SetParticlesInGridsHashing();
    UpdateDensityandPressureGrid();
    UpdatePressureAccelerationGrid();
    updateParticle(dt);
    resetHashLookupTable();
}

void SPH::Draw(sf::RenderWindow& window)
{
    std::for_each(std::execution::par, particles, particles+numParticles , [this](const particle& p) {
        std::size_t index = &p - particles;
        points[index].position = p.Position;
        points[index].color = sf::Color::Blue;
        });

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
    sf::Vector2f offsetVec(0,0);
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

    auto updateParticleProperties = [&](particle& p) {
        p.Acceleration = sf::Vector2f();
        p.Velocity += p.PressureAcceleration * dt;
        if (gravityEnabled)
            p.Acceleration += gravity;
        p.Velocity += p.Acceleration * dt;
        p.Position += p.Velocity * dt;
        if (p.Position.y >= fence.bottom - particleRadius && p.Velocity.y > 0) {
            p.Position.y = fence.bottom - (0.0001f + particleRadius);
            p.Velocity.y = -p.Velocity.y * dampingRate;
        }
        if (p.Position.y <= fence.top + particleRadius && p.Velocity.y < 0) {
            p.Position.y = fence.top + (0.0001f + particleRadius);
            p.Velocity.y = -p.Velocity.y * dampingRate;
        }
        if (p.Position.x >= fence.right - particleRadius && p.Velocity.x > 0) {
            p.Position.x = fence.right - (0.0001f + particleRadius);
            p.Velocity.x = -p.Velocity.x * dampingRate;
        }
        if (p.Position.x <= fence.left + particleRadius && p.Velocity.x < 0) {
            p.Position.x = fence.left + (0.0001f + particleRadius);
            p.Velocity.x = -p.Velocity.x * dampingRate;
        }
        p.PredictedPosition = p.Position + p.Velocity * (1 / 30.0f) + 0.5f * p.Acceleration * (1 / 30.0f) * (1 / 30.0f);

        p.PredictedPosition.x = std::clamp(p.PredictedPosition.x, (float)fence.left, (float)fence.right);
        p.PredictedPosition.y = std::clamp(p.PredictedPosition.y, (float)fence.top, (float)fence.bottom);
        };

    std::for_each(std::execution::par_unseq,
        particles, particles + numParticles,
        updateParticleProperties);

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

    auto calculateDensityAndPressure = [&](particle& p) {
        p.density = calcDensityGrid(&p - particles, p.GridPos);
        p.pressure = ConvertDensityToPressure(p.density);
        };

    std::for_each(std::execution::par_unseq,
        particles, particles + numParticles,
        calculateDensityAndPressure);

}

void SPH::UpdatePressureAccelerationGrid()
{
    auto calculatePressureAcceleration = [&](particle& p) {
        int i = &p - particles;
        particles[i].PressureAcceleration = calcPressureForceGrid(i, particles[i].GridPos);
        };

    std::for_each(std::execution::par_unseq,
        particles, particles + numParticles,
        calculatePressureAcceleration);

}

void SPH::SetParticlesInGridsHashing()
{
    std::for_each(std::execution::par_unseq,
        particles, particles + numParticles,
        [&](particle& p) {
            int gridX = p.PredictedPosition.x / smoothingRadius;
            int gridY = p.PredictedPosition.y / smoothingRadius;
            p.Gridhash = cellHash(gridX, gridY);
            p.GridPos = sf::Vector2f(gridX, gridY);
        });

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


