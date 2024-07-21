# FluidSim2D

FluidSim2D is a fluid simulation project using SFML for graphical rendering. This document provides instructions for setting up and building the project.

## Prerequisites

To build this project, you need the following software:

- **CMake**: Version 3.16 or higher.
- **C++ Compiler**: A modern C++ compiler (e.g., GCC, Clang, MSVC).
- **SFML**: Simple and Fast Multimedia Library (included via FetchContent).

### Installing Dependencies

Ensure you have the required tools installed:

- **CMake**: [Download and install CMake](https://cmake.org/download/).
- **C++ Compiler**:
  - **Windows**: Install Visual Studio or MinGW.
  - **Linux**: Install GCC or Clang using your package manager.
  - **macOS**: Install Xcode or command-line tools.

## Building the Project

Follow these steps to build the project:

1. **Clone the Repository**

    ```sh
    git clone https://github.com/yourusername/FluidSim2D.git
    cd FluidSim2D
    ```

2. **Create a Build Directory**

    ```sh
    mkdir build
    cd build
    ```

3. **Configure the Project with CMake**

    ```sh
    cmake ..
    ```

4. **Build the Project**

    ```sh
    cmake --build .
    ```

    On Windows, you may alternatively use:

    ```sh
    cmake --build . --config Release
    ```

5. **Run the Executable**

    The built executable will be located in the `bin` directory. You can run it as follows:

    ```sh
    ./bin/FluidSim2D
    ```

## Project Structure

- `FluidSim2D/` - Contains source files and headers.
  - `main.cpp` - Entry point of the application.
  - `particle.cpp`, `particle.h` - Defines the particle class for simulation.
  - `particlePhysics.cpp`, `particlePhysics.h` - Handles particle physics.
  - `SimWindow.cpp`, `SimWindow.h` - Manages the simulation window.
  - `Timer.cpp`, `Timer.h` - Manages timing and frame rate.

- `CMakeLists.txt` - The CMake configuration file for the project.
- `.gitignore` - Specifies files and directories to be ignored by Git.
- `README.md` - This file.

## Troubleshooting

If you encounter issues, ensure that:

- All dependencies are correctly installed.
- CMake and the C++ compiler are properly configured.
- You have run CMake from the correct directory.

For further assistance, please check the [SFML documentation](https://www.sfml-dev.org/documentation/2.6.0/) or consult the relevant compiler and CMake documentation.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Contact

For questions or contributions, please open an issue or contact [your email or GitHub profile].
