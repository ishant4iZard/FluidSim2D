#include "SFML/Graphics.hpp"
#include "SimWindow.h"

int main()
{
    srand(time(NULL));
    SimWindow window(800, 800, "2D Fluid Sim");

    while (window.Update())
    {

    }

    return 0;
}