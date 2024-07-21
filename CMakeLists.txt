cmake_minimum_required(VERSION 3.16)
project(CMakeSFMLProject LANGUAGES CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)

include(FetchContent)
FetchContent_Declare(SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.x)
FetchContent_MakeAvailable(SFML)

add_executable(FluidSim2D 
    FluidSim2D/main.cpp
    FluidSim2D/particle.cpp
    FluidSim2D/particle.h
    FluidSim2D/ParticlePhysics.cpp
    FluidSim2D/ParticlePhysics.h
    FluidSim2D/SimWindow.cpp
    FluidSim2D/SimWindow.h
    FluidSim2D/Timer.cpp
    FluidSim2D/Timer.h
)

target_include_directories(FluidSim2D PRIVATE FluidSim2D)

target_link_libraries(FluidSim2D PRIVATE sfml-graphics sfml-window sfml-system sfml-audio)
target_compile_features(FluidSim2D PRIVATE cxx_std_17)

if(WIN32)
    add_custom_command(
        TARGET FluidSim2D
        COMMENT "Copy OpenAL DLL"
        PRE_BUILD COMMAND ${CMAKE_COMMAND} -E copy ${SFML_SOURCE_DIR}/extlibs/bin/$<IF:$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>,x64,x86>/openal32.dll $<TARGET_FILE_DIR:FluidSim2D>
        VERBATIM)
endif()