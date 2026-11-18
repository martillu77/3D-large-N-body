

cmake_minimum_required(VERSION 3.16)

project(large-N-body
    VERSION 0.1.0
    LANGUAGES CXX
)

option(DEBUG_FLAG "Enable debug output" OFF)
if(DEBUG_FLAG)
    add_compile_definitions(DEBUG_FLAG=1)
else()
    add_compile_definitions(DEBUG_FLAG=0)
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
    src/SnapshotWriter.cpp
    src/CheckpointWriter.cpp
    src/CheckpointReader.cpp
    )

find_package(Threads REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(HDF5 REQUIRED COMPONENTS CXX)

target_include_directories(large-N-body PRIVATE
    src
    ${HDF5_INCLUDE_DIRS}
)

target_link_libraries(large-N-body PRIVATE
    Threads::Threads
    ${HDF5_CXX_LIBRARIES}
    ${HDF5_LIBRARIES}
    nlohmann_json::nlohmann_json
)
