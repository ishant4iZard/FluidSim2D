#include "SimWindow.h"
#include <iostream>

SimWindow::SimWindow(int width, int height, const char* name, int innumParticles)
{
	window.create(sf::VideoMode(width, height), name);
    particleRadius = 4.0f;
    numParticles = innumParticles;
    water = new particle[numParticles];
    particleSpacing = 3.2f;
    
    int particlesPerRow = (int)sqrt(numParticles);
    int particlesPerCol = (numParticles - 1) / particlesPerRow + 1;
    float spacing = particleRadius * 2 + particleSpacing;
    for (int i = 0; i < numParticles; i++) {
        float x = (i % particlesPerRow - (particlesPerRow / 2.0f)) * spacing + width/2;
        float y = (i / particlesPerRow - (particlesPerCol / 2.0f)) * spacing + height/2;
        water[i].setPosition(sf::Vector2f(x, y));
    }
    //water.setPosition(sf::Vector2f(width / 2, height / 2));
    //water.setPosition(sf::Vector2f(2, 2));
}

bool SimWindow::Update(double dt)
{
	sf::Event event;
	if (window.pollEvent(event)) {
        switch (event.type) {
        case sf::Event::Closed:
            window.close();
            return false;
            break;
        case sf::Event::KeyPressed:
            if (event.key.code == sf::Keyboard::Escape) {
                window.close();
                return false;
                break;
            }
        }
    }
    dTOffset += dt;

    int iteratorCount = 0;
    while (dTOffset > realDT) {
        for (int i = 0; i < numParticles; i++) {
            water[i].Update(realDT);
        }
        dTOffset -= realDT;
        iteratorCount++;
    }
    if(iteratorCount>0)
        ClearForces();

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

    window.clear();
    for (int i = 0; i < numParticles; i++) {
        window.draw(water[i].drawable());
    }
    window.display();

    timer.Tick();

	return true;
}

void SimWindow::ClearForces()
{
    for (int i = 0; i < numParticles; i++) {
        water[i].ClearForces();
    }
}
