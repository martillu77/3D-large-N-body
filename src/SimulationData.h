#ifndef SIMULATION_DATA_H
#define SIMULATION_DATA_H

#include "Particle.h"
#include <vector>

struct SimulationData {
    std::vector<Particle> particles;
    double dt = 0.;         // Time step for the simulation, calculated from the minimum dynamical time and eta_dt
    double eta_dt = 0.01;   // Configuration.h : Safety coeficient when choosing the simulation's time step: time_step = eta_dt * min_dynamical_time (SimpleVerletEngine)
    double time = 0.0;      // Current simulation time
};

#endif