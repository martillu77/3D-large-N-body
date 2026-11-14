#ifndef SIMULATION_STATE_H
#define SIMULATION_STATE_H

#include "Particle.h"

#include <cstdint>
#include <vector>

struct Snapshot {
    std::uint64_t step;
    double time;
    std::vector<Particle> particles;
};

struct Checkpoint {
    std::uint64_t step;
    double time;
    double dt;
    std::vector<Particle> particles;
    // Més endavant: acceleracions i estat intern de l'integrador.
};

#endif