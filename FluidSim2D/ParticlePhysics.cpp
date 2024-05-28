#include "ParticlePhysics.h"
#include <iostream>
#include "SFML/Window.hpp"
#include <execution>
#include <algorithm>
#include <omp.h>

SPH::SPH(int inNumParticles, float screenWidth, float screenHeight)
{
    numParticles = inNumParticles;
    particles = new particle[numParticles];

    particleRadius = 0.25f;
    shape.setRadius(particleRadius);
    smoothingRadius = 10.0f;
    particleSpacing = 0.125f;
    //threadpoolptr = new ThreadPool(4);

    HorGrids = screenWidth / smoothingRadius;
    VerGrids = screenHeight / smoothingRadius;

    for (int i = 0; i <= HorGrids; i++) {
        std::vector<std::unique_ptr<Grid>> GridRow;
        for (int j = 0; j <= VerGrids; j++) {
            GridRow.emplace_back(new Grid()); // Construct unique_ptr in place
        }
        gridsys.push_back(std::move(GridRow)); // Move GridRow into gridsys
    }

    for (int i = 0; i <= HorGrids; i++) {
        for (int j = 0; j <= VerGrids; j++) {
            gridsys[i][j]->offsetGrids = findOffsetGrids(sf::Vector2f(i, j));
        }
    }

    gridMutexes = new std::mutex * [HorGrids + 1];
    for (int i = 0; i <= HorGrids; ++i) {
        gridMutexes[i] = new std::mutex[VerGrids + 1];
    }

    GridStart(screenWidth, screenHeight);
    //randomPositionStart(screenWidth, screenHeight);

    SetParticlesInGrids();
    UpdateDensityandPressureGrid();
    UpdatePressureAccelerationGrid();
    ClearGrids();
}

SPH::~SPH()
{
    for (int i = 0; i <= HorGrids; ++i) {
        delete[] gridMutexes[i];
    }
    delete[] gridMutexes;

    //delete threadpoolptr;
}

void::SPH::Update(float dt) {
    SetParticlesInGrids();
    UpdateDensityandPressureGrid();
    UpdatePressureAccelerationGrid();
    updateParticle(dt);
    ClearGrids();
    //PhysicsUpdate(dt);
}

void SPH::Draw(sf::RenderWindow& window)
{
    for (int i = 0; i < numParticles; i++) {
        shape.setPosition(particles[i].Position);
        if (particles[i].pressure > 0) {
            shape.setFillColor(sf::Color::Red);
        }
        else if(particles[i].pressure < 0){
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
        particles[i].Position = sf::Vector2f(x, y);
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
        particles[i].Position = sf::Vector2f(x, y);
    }
}

void SPH::PhysicsUpdate(float dt)
{
    dTOffset += dt;

    int iteratorCount = 0;
    while (dTOffset > realDT) {
        /*threadpoolptr->enqueue([this]() {this->SetParticlesInGrids(); });
        threadpoolptr->enqueue([this]() {this->UpdateDensityandPressureGrid(); });
        threadpoolptr->enqueue([this]() {this->UpdatePressureAccelerationGrid(); });
        threadpoolptr->enqueue([this]() {this->updateParticle(realDT); });*/
        
        
        /*SetParticlesInGrids();
        UpdateDensityandPressureGrid();
        UpdatePressureAccelerationGrid();
        updateParticle(realDT);
        ClearGrids();*/

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
    if(useOpenMp)
    {
    #pragma omp parallel for
        for (int i = 0; i < numParticles; ++i) {
            particle& p = particles[i];
            p.Acceleration = p.Force / mass;
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
        }
    }
    else {
        auto updateParticleProperties = [&](particle& p) {
            p.Acceleration = p.Force / mass;
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
        };

        std::for_each(std::execution::par_unseq,
            particles, particles + numParticles,
            updateParticleProperties);
    }
    /*for (int i = 0; i < numParticles; i++) {

        particles[i].Acceleration= particles[i].Force/ mass;
        particles[i].Velocity+= particles[i].PressureAcceleration * dt;
        if (gravityEnabled)
            particles[i].Acceleration += gravity;
        particles[i].Velocity += particles[i].Acceleration * dt;
        particles[i].Position += particles[i].Velocity * dt;
        if (particles[i].Position.y >= fence.bottom - particleRadius && particles[i].Velocity.y > 0) {
            particles[i].Position.y = fence.bottom;
            particles[i].Velocity.y = -particles[i].Velocity.y * dampingRate;
        }
        if (particles[i].Position.y <= fence.top + particleRadius && particles[i].Velocity.y < 0) {
            particles[i].Position.y = fence.top;
            particles[i].Velocity.y = -particles[i].Velocity.y * dampingRate;
        }
        if (particles[i].Position.x >= fence.right - particleRadius && particles[i].Velocity.x > 0) {
            particles[i].Position.x = fence.right;
            particles[i].Velocity.x = -particles[i].Velocity.x * dampingRate;
        }
        if (particles[i].Position.x <= fence.left + particleRadius && particles[i].Velocity.x < 0) {
            particles[i].Position.x = fence.left;
            particles[i].Velocity.x = -particles[i].Velocity.x * dampingRate;
        }
    }*/
}

//float SPH::calcDensity(int particleIndex)
//{
//    float density = 0;
//
//    for (int i = 0; i < numParticles; i++) {
//        float dst = vectorMagnitude(particles[i].Position - particles[particleIndex].Position);
//        float influence = smoothingKernel(smoothingRadius, dst);
//        density += mass * influence;
//    }
//
//    return density;
//}
//
//sf::Vector2f SPH::calcPressureForce(int particleIndex)
//{
//    sf::Vector2f pressureForce = sf::Vector2f(0,0);
//
//    for (int i = 0; i < numParticles; i++) {
//        if (particleIndex == i) continue;
//
//        sf::Vector2f offset(particles[i].Position - particles[particleIndex].Position);
//
//        float dst = vectorMagnitude(offset);
//        if (dst > smoothingRadius) continue;
//        sf::Vector2f dir = dst == 0 ? GetRandomDir() : (offset) / dst;
//        float m_slope = smoothingKernerDerivative(smoothingRadius, dst);
//        float m_density = particles[i].density;
//        float sharedPressure = (particles[i].pressure + particles[particleIndex].pressure)/2;
//        pressureForce += dir * sharedPressure * m_slope * mass / m_density;
//    }
//
//    return pressureForce;
//}

float SPH::calcDensityGrid(int particleIndex, sf::Vector2f gridPos)
{
    float density = 0;
    
    std::vector<sf::Vector2f> offsets = gridsys[gridPos.x][gridPos.y]->offsetGrids;

    for (auto offset : offsets) {
        for (auto it : gridsys[gridPos.x + offset.x][gridPos.y + offset.y]->gridParticles) {
            float dst = vectorMagnitude(particles[it].Position - particles[particleIndex].Position);
            float influence = smoothingKernel(smoothingRadius, dst);
            density += mass * influence;
        }
    }
    return density;
}

sf::Vector2f SPH::calcPressureForceGrid(int particleIndex, sf::Vector2f gridPos)
{
    sf::Vector2f pressureForce = sf::Vector2f(0, 0);
    std::vector<sf::Vector2f> offsets = gridsys[gridPos.x][gridPos.y]->offsetGrids;

    for (auto offset : offsets) {
        for (auto it : gridsys[gridPos.x + offset.x][gridPos.y + offset.y]->gridParticles) {
            if (particleIndex == it) continue;

            sf::Vector2f offsetvec(particles[it].Position - particles[particleIndex].Position);

            float dst = vectorMagnitude(offsetvec);
            if (dst > smoothingRadius) continue;
            sf::Vector2f dir = dst == 0 ? GetRandomDir() : (offsetvec) / dst;
            float m_slope = smoothingKernerDerivative(smoothingRadius, dst);
            float m_density = particles[it].density;
            float sharedPressure = (particles[it].pressure + particles[particleIndex].pressure) / 2;
            pressureForce += dir * sharedPressure * m_slope * mass / m_density;
        }
    }

    return pressureForce;
}

//void SPH::UpdateDensityandPressure() {
//    for (int i = 0; i < numParticles; i++) {
//        particles[i].density = calcDensity(i);
//        particles[i].pressure = ConvertDensityToPressure(particles[i].density);
//    }
//}
//
//void SPH::UpdatePressureAcceleration() {
//    for (int i = 0; i < numParticles; i++) {
//        particles[i].PressureAcceleration = calcPressureForce(i) / particles[i].density;
//    }
//}

void SPH::UpdateDensityandPressureGrid()
{
    if(useOpenMp)
    {
    #pragma omp parallel for
        for (int i = 0; i < numParticles; ++i) {
            particle& p = particles[i];
            p.density = calcDensityGrid(i, p.GridPos);
            p.pressure = ConvertDensityToPressure(p.density);
        }
    }
    else {
        auto calculateDensityAndPressure = [&](particle& p) {
            p.density = calcDensityGrid(&p-particles, p.GridPos); // Assuming calcDensityGrid takes a particle object
            p.pressure = ConvertDensityToPressure(p.density);
            };

        std::for_each(std::execution::par_unseq,
            particles, particles + numParticles,
            calculateDensityAndPressure);
    }
    /*for (int i = 0; i < numParticles; i++) {
        particles[i].density = calcDensityGrid(i, particles[i].GridPos);
        particles[i].pressure = ConvertDensityToPressure(particles[i].density);
    }*/
}

void SPH::UpdatePressureAccelerationGrid()
{
    if(useOpenMp)
    {
    #pragma omp parallel for
        for (int i = 0; i < numParticles; ++i) {
            particle& p = particles[i];
            p.PressureAcceleration = calcPressureForceGrid(i, p.GridPos) / p.density;
        }
    }
    else
    {
        auto calculatePressureAcceleration = [&](particle& p) {
            int i = &p - particles;
            particles[i].PressureAcceleration = calcPressureForceGrid(i, particles[i].GridPos) / particles[i].density;
        };

        std::for_each(std::execution::par_unseq,
            particles, particles + numParticles,
            calculatePressureAcceleration);
    }
    /*for (int i = 0; i < numParticles; i++) {
        particles[i].PressureAcceleration = calcPressureForceGrid(i, particles[i].GridPos) / particles[i].density;
    }*/
}

void SPH::SetParticlesInGrids()
{
    if(useOpenMp)
    {
    #pragma omp parallel for
        for (int i = 0; i < numParticles; ++i) {
            particle& p = particles[i];
            int gridX = static_cast<int>(p.Position.x / smoothingRadius);
            int gridY = static_cast<int>(p.Position.y / smoothingRadius);

            // Lock the mutex for the grid cell being accessed
            {
                std::lock_guard<std::mutex> lock(gridMutexes[gridX][gridY]);
                gridsys[gridX][gridY]->gridParticles.push_back(i);
            }

            p.GridPos = sf::Vector2f(gridX, gridY);
        }
    }

    else
    {
        std::for_each(std::execution::par_unseq,
            particles, particles + numParticles,
            [&](particle& particle) {
                int gridX = particle.Position.x / smoothingRadius;
                int gridY = particle.Position.y / smoothingRadius;
                {
                    // Lock the mutex for the grid cell being accessed
                    std::lock_guard<std::mutex> lock(gridMutexes[gridX][gridY]);
                    gridsys[gridX][gridY]->gridParticles.push_back(&particle - particles);
                }
                particle.GridPos = sf::Vector2f(gridX, gridY);
            });
    }

    /*for (int i = 0; i < numParticles; i++) {
        int gridX = particles[i].Position.x / smoothingRadius;
        int gridY = particles[i].Position.y / smoothingRadius;

        gridsys[gridX][gridY]->gridParticles.push_back(i);
        particles[i].GridPos = sf::Vector2f(gridX, gridY);
    }*/
}

void SPH::ClearGrids()
{
    if (useOpenMp) {
#pragma omp parallel for
        for (int i = 0; i < gridsys.size(); ++i) {
            for (int j = 0; j < gridsys[i].size(); ++j) {
                gridsys[i][j]->gridParticles.clear();
            }
        }
    }
    else {
        auto clearGridParticles = [](std::unique_ptr<Grid>& gridPtr) {
            gridPtr->gridParticles.clear();
            };

        std::for_each(std::execution::par_unseq,
            gridsys.begin(), gridsys.end(),
            [&](std::vector<std::unique_ptr<Grid>>& row) {
                std::for_each(row.begin(), row.end(), clearGridParticles);
            });
    }
    /*for (int i = 0; i < gridsys.size(); i++) {
        for (int j = 0; j < gridsys[i].size(); j++) {
            gridsys[i][j]->gridParticles.clear();
        }
    }*/
}

std::vector<sf::Vector2f> SPH::findOffsetGrids(sf::Vector2f gridPos)
{
    std::vector<sf::Vector2f> offsets;

    offsets.push_back(sf::Vector2f(0, 0));
    if (gridPos.x > 0) {
        offsets.push_back(sf::Vector2f(-1, 0));
        if (gridPos.y > 0) {
            offsets.push_back(sf::Vector2f(-1, -1));
        }
        if (gridPos.y < gridsys[0].size() - 1) {
            offsets.push_back(sf::Vector2f(-1, 1));
        }
    }
    if (gridPos.x < gridsys.size() - 1) {
        offsets.push_back(sf::Vector2f(1, 0));
        if (gridPos.y > 0) {
            offsets.push_back(sf::Vector2f(1, -1));
        }
        if (gridPos.y < gridsys[0].size() - 1) {
            offsets.push_back(sf::Vector2f(1, 1));
        }
    }
    if (gridPos.y > 0) {
        offsets.push_back(sf::Vector2f(0, -1));
    }
    if (gridPos.y < gridsys[0].size() - 1) {
        offsets.push_back(sf::Vector2f(0, 1));
    }
    return offsets;
}

