#include "SimpleVerletEngine.h"
#include "GravitySolver.h"
#include "cstdio" //printf
#include "Particle.h"
#include <cstdint>
#include <vector>

#include <cmath>
#include <cstddef>
#include "cstdio"

SimpleVerletEngine::SimpleVerletEngine() {
}


double SimpleVerletEngine::estimate_snapshot_interval( const SimulationData& data, double snapshot_dist_fraction_ ) const {
    
    if (data.particles.empty()) return 0.0;

    double total_mass = 0.0;
    Vector3 r_cm{};
    Vector3 v_cm{};

    for (const auto& p : data.particles) {
        total_mass += p.mass;
        r_cm += p.position * p.mass;
        v_cm += p.velocity * p.mass;
    }

    if (total_mass <= 0.0)
        return 0.0;

    r_cm = r_cm / total_mass;
    v_cm = v_cm / total_mass;

    double radius_squared = 0.0;
    double max_velocity_squared = 0.0;

    for (const auto& p : data.particles) {
        const auto dr = p.position - r_cm;
        const auto dv = p.velocity - v_cm;

        radius_squared += p.mass * dr.dot(dr);
        max_velocity_squared = std::max(max_velocity_squared, dv.dot(dv));
    }

    const double rms_radius =
        std::sqrt(radius_squared / total_mass);

    const double max_velocity = std::sqrt(max_velocity_squared);

    if (max_velocity <= 0.0)
        return std::numeric_limits<double>::max();

    return snapshot_dist_fraction_ * rms_radius / max_velocity;
}

void SimpleVerletEngine::initialize(SimulationData& data) {
    const auto [accelerations, min_dynamical_time] = gravity::compute_accelerations(data.particles);

    data.dt = data.eta_dt * min_dynamical_time;
    std::printf("eta_dt factor %.3e and minimum dynamical time %.10e : simulation delta t step calculated to be %.10e\n", data.eta_dt, min_dynamical_time, data.dt);

    for (std::size_t i = 0; i < data.particles.size(); ++i) {
        data.particles[i].acceleration = accelerations[i];
    }

}

void SimpleVerletEngine::step(SimulationData& data) {
    const double dt = data.dt;
    const double half_dt = 0.5 * dt;
    const double half_dt2 = half_dt * dt;

    // 1. Update positions with the current acceleration:
    for (auto& p : data.particles) {
        p.position.x += p.velocity.x * dt + half_dt2 * p.acceleration.x;
        p.position.y += p.velocity.y * dt + half_dt2 * p.acceleration.y;
        p.position.z += p.velocity.z * dt + half_dt2 * p.acceleration.z;
    }

    // 2. Calculate accelerations at the new positions.
    const auto [new_accelerations, min_dynamical_time] = gravity::compute_accelerations(data.particles);

    // 3. Update velocities and accelerations with the new accelerations:
    for (std::size_t i = 0; i < data.particles.size(); ++i) {
        auto& p = data.particles[i];
        const auto& a_new = new_accelerations[i];

        p.velocity.x += half_dt * (p.acceleration.x + a_new.x);
        p.velocity.y += half_dt * (p.acceleration.y + a_new.y);
        p.velocity.z += half_dt * (p.acceleration.z + a_new.z);

        p.acceleration = a_new;
    }
}

void SimpleVerletEngine::reset(SimulationData& data) {
    initialize(data);
}