#include "SFML/Graphics.hpp"
#include "SimWindow.h"
#include "Timer.h"

#include <iostream>

int main()
{
    srand(time(NULL));
    SimWindow window(1280, 720, "2D Fluid Sim");

    //GameTimer timer;

    double dt = window.getTimer().GetTimeDeltaSeconds();

    while (window.Update(dt))
    {
        dt = window.getTimer().GetTimeDeltaSeconds();
    }

    return 0;
}