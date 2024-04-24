#include "SimWindow.h"

SimWindow::SimWindow(int width, int height, const char* name, int numParticles)
{
	window.create(sf::VideoMode(width, height), name);
    circle.setRadius(2.0f);
    circle.setFillColor(sf::Color::Blue);
    circle.setPosition(sf::Vector2f(width / 2, height / 2));
}

bool SimWindow::Update()
{
	sf::Event event;
	while (window.pollEvent(event)) {
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


    window.clear();
    window.draw(circle);
    window.display();

	return true;
}
