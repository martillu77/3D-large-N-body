#ifndef INITIAL_CONDITIONS_H
#define INITIAL_CONDITIONS_H

#include "Particle.h"
#include "Configuration.h"

#include <cstddef>
#include <cstdint>
#include <vector>

struct SimulationUnits {
    double mass_scale;      // kg per unitat interna de massa
    double length_scale;    // m per unitat interna de distància
    double time_scale;      // s per unitat interna de temps
};

class InitialConditions {

public:
    static std::vector<Particle> generate(const Configuration& config, SimulationUnits& units);

private:

// For debugging purposes:
    static std::vector<Particle> generate_Twin_Masses();

    static std::vector<Particle> generate_Threebody_Lagrange();

    static std::vector<Particle> generate_Solar_system();


// Realistic or physics based cases:
    static std::vector<Particle> generate_rotating_disk(
        std::size_t N,
        Vector3 center,
        double R,
        double omega,
        double mass = 1.0,
        double radius_scale = 0.05,
        std::uint64_t seed = 1
    );

    static std::vector<Particle> generate_proto_disc(
        std::size_t N,
        Vector3 center, double R_outer, double stellar_mass,
        double disk_mass_fraction, double lognormal_sigma,
        double omega,
        double radius_scale = 0.05,
        std::uint64_t seed = 1
    );

    static std::vector<Particle> generate_collapsing_cloud(
        std::size_t N,
        Vector3 center,
        double R_outer, double cloud_mass,
        double omega,
        double rotational_bias,
        double radius_scale = 0.05,
        std::uint64_t seed = 1
    );

    static std::vector<Particle> generate_collapsing_cloud_lognormal(
        std::size_t N,
        Vector3 center,
        double R_outer, double cloud_mass,
        double omega,
        double rotational_bias,
        double sigma,
        double radius_scale = 0.05,
        std::uint64_t seed = 1
    );
};

#endif