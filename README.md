# Large N-body Simulator

A 3D gravitational N-body simulator written in C++20, with a Python interface.

The project aims to simulate the dynamical evolution of systems of particles interacting through gravity 
The C++ simulation engine is designed to be independent of the Python interface, allowing the 
numerical simulation and its visualisation to evolve separately.

**Current version:** v0.0.1  
**Status:** Early development

## Features

- Three-dimensional Newtonian gravitational interactions.
- Direct CPU gravity solver with \(O(N^2)\) computational complexity.
- Velocity Verlet integration.
- Initial conditions for several types of systems:
  - Rotating disk.
  - Protoplanetary disk.
  - Collapsing cloud.
  - Collapsing cloud with a lognormal mass distribution.
  - For debugging purposes there are also:
  	- 
  	-
  	-
- Configurable simulation parameters through card files where, for now, units are in SI.
- Characteristic units for mass, length and time, with internal gravitational constant \(G=1\).
- Python interface for controlling the simulation.

## Requirements

- C++ compiler supporting C++20.
- CMake 3.16 or later.
- Python 3 for the interface.

## Building

From the project root directory:

```bash
cmake -S . -B build
cmake --build build -j
```

The executable is generated at:

```text
build/large-N-body
```

To perform a clean build:

```bash
rm -rf build
cmake -S . -B build
cmake --build build -j
```

## Configuration

Simulation parameters and initial conditions are specified in card files stored in the `config/` directory.

The repository includes `config/test.card` as an example configuration.

## Project structure

```text
large-N-body/
├── CMakeLists.txt
├── README.md
├── config/       # Simulation configuration files
├── interface/    # Python interface
└── src/          # C++ simulation engine
```

## Current limitations

This is an early development version. The implementation and numerical behaviour have been tested on a small 
number of systems, but the simulator has not undergone yet comprehensive numerical validation.

Current limitations include:

- Direct gravitational calculations scale quadratically with the number of particles.
- No collision or merger model.
- No gravitational softening.
- Visualisation and diagnostic tools are still under development.
- Numerical accuracy and long-term stability depend on the integration parameters and the simulated system.

## Future development

Planned improvements include:

- Visualisation of particle positions and trajectories.
- Monitoring of kinetic, potential and total energy, and angular momentum.
- Mass distribution diagnostics.
- Improved adaptive time-stepping.
- More efficient gravity solvers for larger particle counts: GPU usage and ray tracing

## License

No license has been specified yet.
