# Large N-body Simulator

A 3D gravitational N-body simulator written in C++20, with two independent Python tools for simulation control and interactive visualisation.

The project simulates the dynamical evolution of systems of particles interacting through gravity. It is organised into three independent components:

- **C++ simulation engine (`large-N-body`):** performs the numerical integration of the gravitational N-body system and stores simulation data in a JSON header and a HDF5 files.
- **Python controller (`control.py`):** provides an interface to configure, start, pause, resume and stop simulations, as well as inspect their status.
- **Python visualiser (`viewer.py`):** reads the HDF5 output and provides interactive 3D visualisation, including snapshot navigation, playback controls, adjustable playback speed and zoom.

The C++ engine operates independently of the Python tools. The controller communicates with the simulation engine, while the visualiser works directly with the resulting HDF5 file. This separation allows the numerical engine, simulation control and visualisation to evolve independently.

**Current version:** v0.0.2
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
  	- Twin_Masses
  	- Threebody_Lagrange
  	- Solar_system
- Configurable simulation parameters through card files where, for now, units are in SI.
- Characteristic units for mass, length and time, with internal gravitational constant \(G=1\).
- Python interface for controlling the simulation.
- Python visualiser for interactive visualisation of the simulation results.

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
- Numerical accuracy and long-term stability depend on the integration parameters and the simulated system.

## Future development

Planned improvements include:

- Monitoring of kinetic, potential and total energy, and angular momentum.
- Mass distribution diagnostics.
- Improved adaptive time-stepping.
- More efficient gravity solvers for larger particle counts: GPU usage and ray tracing


## Simulation Controller

The Python controller (`control.py`) provides a command-line interface to interact with the C++ simulation engine.

Run it from the project root:
> python3 interface/control.py

The controller accepts the following commands:

| Command                  | Description
| `load_config <filename>` | Load simulation parameters from a configuration file.
| `start`                  | Start the simulation.
| `pause`                  | Pause the running simulation.
| `resume`                 | Resume a paused simulation.
| `stop`                   | Stop the simulation.
| `status`                 | Display the current simulation state.
| `dump`                   | Display detailed simulation information.
| `help`                   | Show the available commands.
| `quit`                   | Exit the controller.

A typical workflow is to load a configuration file, start the simulation and monitor its progress using `status` and `dump`. The simulation can be paused, resumed or stopped at any time BUT not saved yet.

The controller and simulation engine operate as separate components, communicating through the control interface.



## Simulation Visualiser

The Python visualiser (`viewer.py`) reads simulation output stored in HDF5 files and displays the particle trajectories as an interactive 3D animation.

Run it from the project root:
> python3 interface/viewer.py <simulation_file.h5>

The visualiser provides the following controls:

| Control                 | Description
| **Play**                | Start or resume playback.
| **Pause**               | Pause playback.
| **FPS slider**          | Adjust the target display update frequency (frames per second).
| **Frame step slider**   | Set the number of snapshots to advance at each update.
| **Snapshot slider**     | Select a specific snapshot.
| **Mouse wheel**         | Zoom in or out, centred on the mouse position.
| **Left / Right arrows** | Move one snapshot backward or forward.
| **Page Up / Page Down** | Jump 100 snapshots forward or backward.
| **Space**               | Toggle playback.
| **r**                   | Reset the zoom to the initial view.
| **q**                   | Close the visualiser.

The FPS and frame step controls can be combined to adjust playback speed. Increasing the frame step skips snapshots during playback, allowing faster navigation through long simulations even when the display update frequency is limited.

The visualiser operates independently of the simulation engine and can be used to inspect completed simulations from their HDF5 output files.

## License

No license has been specified yet.
