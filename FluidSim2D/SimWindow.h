#pragma once
#include "SFML/Graphics.hpp"
#include "SFML/Window.hpp"

class SimWindow 
{
private:
	sf::RenderWindow window;
	sf::CircleShape circle;

public:
	SimWindow(int width, int height, const char* name,int numParticles =100);
	bool Update();
};