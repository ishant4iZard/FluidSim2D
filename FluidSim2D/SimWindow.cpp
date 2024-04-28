#include "SimWindow.h"
#include <iostream>

SimWindow::SimWindow(int width, int height, const char* name, int innumParticles)
{
	window.create(sf::VideoMode(width, height), name);
    numParticles = innumParticles;
    water = new SPH(100000,width,height);
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
        case sf::Event::MouseButtonPressed:
            if (event.mouseButton.button == sf::Mouse::Button::Left) {
                float pX, pY;
                pX = event.mouseButton.x;
                pY = event.mouseButton.y;
                float density = water->calcDensity(sf::Vector2f(pX, pY));
                std::cout << density<<"\n";
            }
        }
    }

    water->Update(dt);
    

    window.clear();
    water->Draw(window);
    window.display();

    timer.Tick();

	return true;
}


