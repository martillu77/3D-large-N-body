#ifndef SIMULATION_DATA_H
#define SIMULATION_DATA_H

#include "Particle.h"
#include <vector>

struct SimulationData {
    std::vector<Particle> particles;
    double dt = 0.;
    double eta_dt = 0.01;
    double time = 0.0;
};

#endif