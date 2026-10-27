#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <cstdint>
#include <string>

class Configuration {
public:
    Configuration();

    bool load(const std::string& filename);

    int particles() const;
    double mass() const;

    double radius() const;
    double distribution_radius() const;

    bool collisions() const;
    double restitution() const;

    const std::string& initial_conditions() const;

    double omega() const;
    double stellar_mass() const;
    double cloud_mass() const;
    double disk_mass_fraction() const;
    double lognormal_sigma() const;
    double rotational_bias() const;
    double sigma() const;

    double eta_dt() const;
    const std::string& model() const;
    bool seed_provided() const;
    std::uint64_t seed() const;

private:
    int particles_;                  // Number of particles
    double mass_;                    // Particle mass
    double radius_;                  // Particle radius

    // Initial-condition parameters
    std::string initial_conditions_; // Initial conditions model
    double distrib_radius_;          // Radius of the distribution (for "disk" and "sphere" distributions)
    double stellar_mass_;
    double cloud_mass_;

    double disk_mass_fraction_;      // Fraction of stellar mass in the disk                             (for proto-disc)
    double lognormal_sigma_;         // Standard deviation of log-normal distribution of particle masses (for proto-disc)

    double omega_;
    double rotational_bias_;
    double sigma_;


    bool collisions_;                // Whether collisions are enabled (true) or not (false)
    double restitution_;             // =1 perfectly elastic collision, =0 perfectly inelastic collision

    double eta_dt_;                  // Safety coeficient when choosing the simulation's time step: time_step = eta_dt * min_dynamical_time (SimpleVerletEngine)
    std::string model_;              // Model name (e.g. "SimpleVerlet")
    bool seed_provided_;
    std::uint64_t seed_;



};

#endif