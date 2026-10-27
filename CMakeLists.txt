

cmake_minimum_required(VERSION 3.16)

project(large-N-body
    VERSION 0.1.0
    LANGUAGES CXX
)

option(DEBUG_FLAG "Enable debug output" ON)
if(DEBUG_FLAG)
    add_compile_definitions(DEBUG_FLAG)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(large-N-body
    src/main.cpp
    src/Configuration.cpp
    src/Simulation.cpp
    src/InitialConditions.cpp
    src/SimpleVerletEngine.cpp
    src/GravitySolverCPU.cpp
    )

target_include_directories(large-N-body PRIVATE src)
