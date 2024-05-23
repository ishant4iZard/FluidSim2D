#include "ParticlePhysics.h"
#include <iostream>
#include "SFML/Window.hpp"

SPH::SPH(int inNumParticles, float screenWidth, float screenHeight)
{
    numParticles = inNumParticles;
    /*Position    = new sf::Vector2f[numParticles];
    Velocity    = new sf::Vector2f[numParticles];
    Force       = new sf::Vector2f[numParticles];
    Acceleration= new sf::Vector2f[numParticles];
    PressureAcceleration = new sf::Vector2f[numParticles];
    density     = new float[numParticles];
    pressure    = new float[numParticles];*/
    particles = new particle[numParticles];

    particleRadius = 2.0f;
    shape.setRadius(particleRadius);
    smoothingRadius = 125.0f;
    particleSpacing = 0.5f;
    //threadpoolptr = new ThreadPool(4);

    int HorGrids = screenWidth / smoothingRadius;
    int VerGrids = screenHeight / smoothingRadius;

    for (int i = 0; i <= HorGrids; i++) {
        std::vector<Grid*> GridRow;
        for (int j = 0; j <= VerGrids; j++) {
            Grid* grid = new Grid;
            GridRow.push_back(grid);
        }
        gridsys.push_back(GridRow);
    }

    GridStart(screenWidth, screenHeight);
    //randomPositionStart(screenWidth, screenHeight);

    //CreateGrids();

    SetParticlesInGrids();
    UpdateDensityandPressureGrid();
    UpdatePressureAccelerationGrid();
    ClearGrids();
}



SPH::~SPH()
{
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
    for (int i = 0; i < numParticles; i++) {

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

    }
}


float SPH::calcDensity(int particleIndex)
{
    float density = 0;

    for (int i = 0; i < numParticles; i++) {
        float dst = vectorMagnitude(particles[i].Position - particles[particleIndex].Position);
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

        sf::Vector2f offset(particles[i].Position - particles[particleIndex].Position);

        float dst = vectorMagnitude(offset);
        if (dst > smoothingRadius) continue;
        sf::Vector2f dir = dst == 0 ? GetRandomDir() : (offset) / dst;
        float m_slope = smoothingKernerDerivative(smoothingRadius, dst);
        float m_density = particles[i].density;
        float sharedPressure = (particles[i].pressure + particles[particleIndex].pressure)/2;
        pressureForce += dir * sharedPressure * m_slope * mass / m_density;
    }

    return pressureForce;
}

float SPH::calcDensityGrid(int particleIndex, sf::Vector2f gridPos)
{
    float density = 0;
    
    std::vector<sf::Vector2f> offsets = findOffsetGrids(gridPos);

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
    std::vector<sf::Vector2f> offsets = findOffsetGrids(gridPos);


    for (auto offset : offsets) {
        for (auto it : gridsys[gridPos.x + offset.x][gridPos.y + offset.y]->gridParticles) {
            if (particleIndex == it) continue;

            sf::Vector2f offset(particles[it].Position - particles[particleIndex].Position);

            float dst = vectorMagnitude(offset);
            if (dst > smoothingRadius) continue;
            sf::Vector2f dir = dst == 0 ? GetRandomDir() : (offset) / dst;
            float m_slope = smoothingKernerDerivative(smoothingRadius, dst);
            float m_density = particles[it].density;
            float sharedPressure = (particles[it].pressure + particles[particleIndex].pressure) / 2;
            pressureForce += dir * sharedPressure * m_slope * mass / m_density;
        }
    }

    return pressureForce;
}


void SPH::UpdateDensityandPressure() {
    for (int i = 0; i < numParticles; i++) {
        particles[i].density = calcDensity(i);
        particles[i].pressure = ConvertDensityToPressure(particles[i].density);
    }
}




void SPH::UpdatePressureAcceleration() {
    for (int i = 0; i < numParticles; i++) {
        particles[i].PressureAcceleration = calcPressureForce(i) / particles[i].density;
    }
}



void SPH::UpdateDensityandPressureGrid()
{
    for (int i = 0; i < numParticles; i++) {
        particles[i].density = calcDensityGrid(i, particles[i].GridPos);
        particles[i].pressure = ConvertDensityToPressure(particles[i].density);
    }
}

void SPH::UpdatePressureAccelerationGrid()
{
    for (int i = 0; i < numParticles; i++) {
        particles[i].PressureAcceleration = calcPressureForceGrid(i, particles[i].GridPos) / particles[i].density;
    }
}

//void SPH::CreateGrids()
//{
//
//}

void SPH::SetParticlesInGrids()
{
    for (int i = 0; i < numParticles; i++) {
        int gridX = particles[i].Position.x / smoothingRadius;
        int gridY = particles[i].Position.y / smoothingRadius;

        gridsys[gridX][gridY]->gridParticles.push_back(i);
        particles[i].GridPos = sf::Vector2f(gridX, gridY);
    }
}

void SPH::ClearGrids()
{
    for (int i = 0; i < gridsys.size(); i++) {
        for (int j = 0; j < gridsys[i].size(); j++) {
            gridsys[i][j]->gridParticles.clear();
        }
    }
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

