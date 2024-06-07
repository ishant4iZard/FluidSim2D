#include "SimWindow.h"
#include <iostream>

#include <chrono>

SimWindow::SimWindow(int width, int height, const char* name)
{
	window.create(sf::VideoMode(width, height), name);
    numParticles = 50000;
    water = new SPH(numParticles,width,height);
}

SimWindow::~SimWindow()
{
    delete water;
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
                
            }
        }
    }

    auto start_time
        = std::chrono::high_resolution_clock::now();
    water->Update(dt);
    
    auto physics_end_time
        = std::chrono::high_resolution_clock::now();

    auto taken_time_physics = std::chrono::duration_cast<
        std::chrono::milliseconds>(
            physics_end_time - start_time)
        .count();

    window.clear();
    water->Draw(window);
    window.display();

    auto render_end_time
        = std::chrono::high_resolution_clock::now();

    auto taken_time_render = std::chrono::duration_cast<
        std::chrono::milliseconds>(
            render_end_time - physics_end_time)
        .count();


    timer.Tick();

    std::cout << "\r"
        << "phy execution time: " << taken_time_physics
        << "ms "
        << "ren execution time: " << taken_time_render
        << "ms "
        << "total time:"<< timer.GetTimeDeltaMSec()
        << "ms ";
    //std::cout << "\r" << 1.0f / timer.GetTimeDeltaSeconds();

	return true;
}


