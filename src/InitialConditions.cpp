#include "InitialConditions.h"

#include "Configuration.h"
#include <stdexcept>

#include <algorithm>
#include <cmath>
#include <numbers>
#include <random>
#include <cstdint>
#include <stdexcept>
#include <vector>

namespace {
    constexpr double G_SI = 6.67430e-11;
}


/////////////////////////////////////////////////////////////////////////
std::vector<Particle> InitialConditions::generate( const Configuration& config, SimulationUnits& units ) {
/////////////////////////////////////////////////////////////////////////
    const auto N = static_cast<std::size_t>(config.particles());
    const auto center = Vector3{0.0, 0.0, 0.0};

    const auto& initial_conditions = config.initial_conditions();


    // Choose the reference mass according to the initial-conditions model:
    double M0 = 0.0;

    if (initial_conditions == "Solar_system") {
        M0 = 1;
    }
    else if (initial_conditions =="Twin_Masses") {
        M0 = 1;
    }
    else if (initial_conditions =="Threebody_Lagrange") {
        M0 = 1;
    }

    else if (initial_conditions == "rotating_disk") {
        M0 = N * config.mass();
    }
    else if (initial_conditions == "proto_disc") {
        // Expected total mass: central star + disk particles.
        // The disk masses follow a lognormal distribution with
        // mu = log(disk_mass_fraction * stellar_mass) and lognormal_sigma = 1.5.

        const double Mstar = config.stellar_mass();
        const double expected_disk_particle_mass =
            config.disk_mass_fraction() * Mstar
            * std::exp(0.5 * config.lognormal_sigma() * config.lognormal_sigma());

        M0 = Mstar + N * expected_disk_particle_mass;
    }
    else if (initial_conditions == "collapsing_cloud" || initial_conditions == "collapsing_cloud_lognormal") {
        M0 = config.cloud_mass();
    }
    else throw std::runtime_error("Unknown initial conditions model: " + initial_conditions);
    
    if (M0 <= 0.0) throw std::runtime_error("Reference mass M0 is not positive.");

    units.mass_scale = M0;



    // Other reference units: length and time scales:
    units.length_scale = config.distribution_radius();
    units.time_scale   = std::sqrt(
                              units.length_scale * units.length_scale * units.length_scale
                              / (G_SI * units.mass_scale));

    // Convert input parameters to internal units, where G = 1.
    const double omega = config.omega() * units.time_scale;
    const double radius_scale = config.radius() / units.length_scale;

    // Reproducible seed if supplied; otherwise generate one.
    std::random_device rd;
    const std::uint64_t seed = config.seed_provided()
        ? config.seed()
        : (static_cast<std::uint64_t>(rd()) << 32) | rd();



    // Initialize the simulation's Universe:

    // Below for debugging purposes:
    if (initial_conditions == "Twin_Masses") {
        return generate_Twin_Masses();
    }

    if (initial_conditions == "Threebody_Lagrange") {
        return generate_Threebody_Lagrange();
    }

    if (initial_conditions == "Solar_system") {
        return generate_Solar_system();
    }

    // Physics oriented initializations:
    if (initial_conditions == "rotating_disk") {
        return generate_rotating_disk( N, center, 1.0, omega, config.mass()/ units.mass_scale, seed, radius_scale );
    }

    if (initial_conditions == "proto_disc") {
        return generate_proto_disc( N, center, 1.0, config.stellar_mass()/ units.mass_scale, 
        config.disk_mass_fraction(), config.lognormal_sigma(), 
        omega, seed, radius_scale );
    }

    if (initial_conditions == "collapsing_cloud") {
        return generate_collapsing_cloud( N, center, 1.0, config.cloud_mass()/ units.mass_scale, 
        omega, config.rotational_bias(), seed, radius_scale );
    }

    if (initial_conditions == "collapsing_cloud_lognormal") {
        return generate_collapsing_cloud_lognormal( N, center, 1.0, config.cloud_mass()/ units.mass_scale, 
        omega, config.rotational_bias(), config.sigma(), seed, radius_scale );
    }

    throw std::runtime_error("Unknown initial conditions : " + initial_conditions);
}




/////////////////////////////////////////////////////////////////////////
namespace {
/////////////////////////////////////////////////////////////////////////

    using RandomEngine = std::mt19937_64;

    ///////////////
    double uniform(RandomEngine& rng){
        return std::generate_canonical<double, 53>(rng);
    }

    ///////////////
    double particle_radius(double mass, double radius_scale) {
        return radius_scale * std::cbrt(mass);
    }

    ///////////////
    Particle make_particle( Vector3 position, Vector3 velocity, double mass, double radius_scale) {
        if (!(mass > 0.0) || !std::isfinite(mass)) throw std::invalid_argument("Particle mass must be positive and finite.");

        Particle p;

        p.position = position;
        p.velocity = velocity;
        p.acceleration = {0.0, 0.0, 0.0};
        p.mass = mass;
        p.radius = particle_radius(mass, radius_scale);

        return p;
    }

    ///////////////
    void validate_common( std::size_t N, double R, double mass, double radius_scale) {
        if (N == 0) throw std::invalid_argument("N must be greater than zero.");

        if (!(R > 0.0) || !std::isfinite(R))
            throw std::invalid_argument("Radius must be positive and finite.");

        if (!(mass > 0.0) || !std::isfinite(mass))
            throw std::invalid_argument("Mass must be positive and finite.");

        if (!(radius_scale > 0.0) || !std::isfinite(radius_scale))
            throw std::invalid_argument("radius_scale must be positive and finite.");
    }

    ///////////////
    void validate_bias(double rotational_bias) {
        if (!std::isfinite(rotational_bias) || rotational_bias < 0.0 || rotational_bias > 1.0) {
            throw std::invalid_argument("rotational_bias must be between 0 and 1.");
        }
    }
}

std::vector<Particle> InitialConditions::generate_Twin_Masses() {

    std::vector<Particle> particles;

    particles.push_back( make_particle( Vector3 { -0.5, 0.0, 0.0 }, Vector3 { 0.0, -0.5, 0.0 }, 0.5, 0.01 ));
    particles.push_back( make_particle( Vector3 { 0.5, 0.0, 0.0 },  Vector3 { 0.0, 0.5, 0.0 },  0.5, 0.01 ));

    return particles;

}

std::vector<Particle> InitialConditions::generate_Threebody_Lagrange() {

    std::vector<Particle> particles;

    // v = sqrt ( G m / r ) = 1/sqrt(3)
    // Body 1
    particles.push_back(make_particle(
    Vector3{ 0.0,  1.0 / std::sqrt(3.0), 0.0 },
    Vector3{ -1.0 / std::sqrt(3.0), 0.0, 0.0 },
    1./3., 0.01));

    // Body 2
    particles.push_back(make_particle(
    Vector3{ -0.5, -1.0 / (2.0 * std::sqrt(3.0)), 0.0 },
    Vector3{ 1.0 / std::sqrt(3.0) / 2.0, -0.5, 0.0 },
    1./3., 0.01));

    // Body 3
    particles.push_back(make_particle(
    Vector3{ 0.5, -1.0 / (2.0 * std::sqrt(3.0)), 0.0 },
    Vector3{ 1.0 / std::sqrt(3.0) / 2.0, 0.5, 0.0 },
    1./3., 0.01));

    return particles;
}

std::vector<Particle> InitialConditions::generate_Solar_system() {

    std::vector<Particle> particles;

    // Sun
    particles.push_back(make_particle(
    Vector3{ 0.0, 0.0, 0.0 },
    Vector3{ 0.0, 0.0, 0.0 },
    1.0, 0.01
    ));

    // Mercury
    particles.push_back(make_particle(
    Vector3{ 0.3871, 0.0, 0.0 },
    Vector3{ 0.0, 1.607, 0.0 },
    1.6601e-7, 0.01
    ));

    // Venus
    particles.push_back(make_particle(
    Vector3{ 0.7233, 0.0, 0.0 },
    Vector3{ 0.0, 1.176, 0.0 },
    2.4478e-6, 0.01
    ));

    // Earth
    particles.push_back(make_particle(
    Vector3{ 1.0000, 0.0, 0.0 },
    Vector3{ 0.0, 1.0000, 0.0 },
    3.0035e-6, 0.01
    ));

    // Mars
    particles.push_back(make_particle(
    Vector3{ 1.5237, 0.0, 0.0 },
    Vector3{ 0.0, 0.810, 0.0 },
    3.2272e-7, 0.01
    ));

    // Jupiter
    particles.push_back(make_particle(
    Vector3{ 5.2044, 0.0, 0.0 },
    Vector3{ 0.0, 0.4384, 0.0 },
    9.5458e-4, 0.01
    ));

    // Saturn
    particles.push_back(make_particle(
    Vector3{ 9.5826, 0.0, 0.0 },
    Vector3{ 0.0, 0.3230, 0.0 },
    2.8581e-4, 0.01
    ));

    return particles;

}

// ============================================================
// Uniform rotating disk
// ============================================================

std::vector<Particle> InitialConditions::generate_rotating_disk(
    std::size_t N,
    Vector3 center,
    double R,
    double omega,
    double mass,
    double radius_scale,
    std::uint64_t seed
    ) {

    validate_common(N, R, mass, radius_scale);

    RandomEngine rng(seed);

    std::vector<Particle> particles;
    particles.reserve(N);

    const double two_pi = 2.0 * std::numbers::pi;

    for (std::size_t i = 0; i < N; ++i){
        // Uniform spatial distribution over the disk area.
        const double r = R * std::sqrt(uniform(rng));
        const double theta = two_pi * uniform(rng);

        const double dx = r * std::cos(theta);
        const double dy = r * std::sin(theta);

        Vector3 position{ center.x + dx, center.y + dy, center.z };

        // Global rotation around the z axis.
        Vector3 velocity{ -omega * dy, omega * dx, 0.0 };

        // Small random velocity perturbation.
        velocity.x += 0.1 * (uniform(rng) - 0.5);
        velocity.y += 0.1 * (uniform(rng) - 0.5);

        particles.push_back( make_particle(position, velocity, mass, radius_scale ));
    }

    return particles;
}


// ============================================================
// Circumstellar protoplanetary disk
// ============================================================

/*
Generates a realistic proto-disc:
   - Central star with mass `stellar_mass` at the center (Sol = 1 M☉)
   - Gas and dust particles 0.2% of the stellar mass, distributed in a disk with radius `R_outer`
   - Log-normal distribution of particle masses 
   - Keplerian rotation around the central star, with small random velocity perturbations
Arguments:
   - N: number of particles (w/o central star)
   - center: position of the central star
   - R_outer: outer radius of the disk
   - stellar_mass: mass of the central star
   - disk_mass_fraction: fraction of the stellar mass in the disk (default 0.002)
   - lognormal_sigma: standard deviation of the log-normal distribution of particle masses (default 1.5)
   - radius_scale: scale factor for particle radii
   - omega: angular velocity at R_outer (Keplerian)
   - seed: random seed for reproducibility
*/
std::vector<Particle> InitialConditions::generate_proto_disc(
    std::size_t N,
    Vector3 center, double R_outer, double stellar_mass,
    double disk_mass_fraction, double lognormal_sigma,
    double omega,
    double radius_scale,
    std::uint64_t seed) {

    validate_common(N, R_outer, stellar_mass, radius_scale);

    RandomEngine rng(seed);

    std::vector<Particle> particles;
    particles.reserve(N + 1);

    // Central star.
    particles.push_back( make_particle( center, {0.0, 0.0, 0.0}, stellar_mass, radius_scale ));

    // Log-normal particle masses.
    // Equivalent to scipy.stats.lognorm(s=1.5, scale=stellar_mass*0.002).
    std::lognormal_distribution<double> mass_dist( std::log(stellar_mass * disk_mass_fraction), lognormal_sigma);

    const double two_pi = 2.0 * std::numbers::pi;
    constexpr double dispersion_fraction = 0.1;

    for (std::size_t i = 0; i < N; ++i) {
        const double r = R_outer * std::sqrt(uniform(rng));
        const double theta = two_pi * uniform(rng);

        const double dx = r * std::cos(theta);
        const double dy = r * std::sin(theta);

        Vector3 position{ center.x + dx, center.y + dy, center.z };

        // Keplerian speed, normalized at R_outer.
        const double v_kepler = omega * R_outer * std::sqrt(R_outer / r);

        Vector3 velocity{
            -v_kepler * std::sin(theta),
             v_kepler * std::cos(theta),
             0.0
        };

        // Small orbital velocity dispersion.
        velocity.x += dispersion_fraction * v_kepler * (uniform(rng) - 0.5);

        velocity.y += dispersion_fraction * v_kepler * (uniform(rng) - 0.5);

        const double mass = mass_dist(rng);

        particles.push_back( make_particle(position, velocity, mass, radius_scale ));
    }

    return particles;
}


// ============================================================
// Uniform-mass collapsing cloud
// ============================================================
/*
Generates a collapsing molecular cloud:
   - Initial phase of star formation
   - No central star
   - Gas and dust particles 0.2% of the stellar mass, distributed in a disk with radius `R_outer`
   - All particles have the same mass, equal to `cloud_mass / N`
   - No Keplerian velocity, but a small random velocity dispersion with a bias towards rotation
Arguments:
   - N: number of particles (w/o central star)
   - center: position of the central star
   - R_outer: outer radius of the disk
   - cloud_mass: total mass of the cloud
   - radius_scale: scale factor for particle radii
   - omega: angular velocity of global rotation (not Keplerian)
   - rotational_bias: fraction of the velocity dispersion that is aligned with the rotation axis (0 = random, 1 = fully aligned)
   - seed: random seed for reproducibility
*/
std::vector<Particle> InitialConditions::generate_collapsing_cloud(
    std::size_t N,
    Vector3 center,
    double R_outer, double cloud_mass,
    double omega,
    double rotational_bias,
    double radius_scale,
    std::uint64_t seed
    ) {

    validate_common(N, R_outer, cloud_mass, radius_scale);
    validate_bias(rotational_bias);

    RandomEngine rng(seed);

    std::vector<Particle> particles;
    particles.reserve(N);

    const double mass_per_particle = cloud_mass / static_cast<double>(N);

    const double dispersion = omega * R_outer;
    const double two_pi = 2.0 * std::numbers::pi;

    for (std::size_t i = 0; i < N; ++i) {
        // Uniform distribution inside a 3D sphere.
        const double r = R_outer * std::cbrt(uniform(rng));
        const double cos_theta = 2.0 * uniform(rng) - 1.0;
        const double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);

        const double phi = two_pi * uniform(rng);

        Vector3 position{
            center.x + r * sin_theta * std::cos(phi),
            center.y + r * sin_theta * std::sin(phi),
            center.z + r * cos_theta
        };

        // Preserve the velocity-component prescription of the
        // original Python implementation.
        Vector3 velocity{
            (uniform(rng) - rotational_bias) * dispersion,
            (uniform(rng) - rotational_bias) * dispersion,
            (uniform(rng) - rotational_bias) * dispersion
        };

        // Random masses around the mean, as in the Python version.
        const double mass = mass_per_particle * (0.01 + 1.99 * uniform(rng));

        particles.push_back( make_particle(position, velocity, mass, radius_scale ));
    }

    return particles;
}


// ============================================================
// Collapsing cloud with log-normal particle masses
// ============================================================
/*
Generates a collapsing molecular cloud:
   - Same as `generate_collapsing_cloud`, but with a log-normal distribution of particle masses
*/
std::vector<Particle>
InitialConditions::generate_collapsing_cloud_lognormal(
    std::size_t N,
    Vector3 center,
    double R_outer, double cloud_mass,
    double omega,
    double rotational_bias,
    double sigma,
    double radius_scale,
    std::uint64_t seed
    ) {

    validate_common(N, R_outer, cloud_mass, radius_scale);
    validate_bias(rotational_bias);

    if (!(sigma > 0.0) || !std::isfinite(sigma))
        throw std::invalid_argument("sigma must be positive and finite.");

    RandomEngine rng(seed);

    std::vector<Particle> particles;
    particles.reserve(N);

    // Generate log-normal relative masses.
    // We normalize them afterwards so their sum equals cloud_mass.
    std::lognormal_distribution<double> mass_dist( std::log(cloud_mass * 0.01), sigma);

    std::vector<double> masses(N);
    double total_mass = 0.0;

    for (double& mass : masses) {
        mass = mass_dist(rng);
        total_mass += mass;
    }

    if (!(total_mass > 0.0) || !std::isfinite(total_mass))
        throw std::runtime_error("Invalid total mass from log-normal distribution.");
    
    // Make sure that, as "advertised", total mass of the particles equals cloud_mass:
    for (double& mass : masses) mass *= cloud_mass / total_mass;

    const double dispersion = omega * R_outer;
    const double two_pi = 2.0 * std::numbers::pi;

    for (std::size_t i = 0; i < N; ++i) {
        // Uniform distribution inside a 3D sphere.
        const double r = R_outer * std::cbrt(uniform(rng));
        const double cos_theta = 2.0 * uniform(rng) - 1.0;
        const double sin_theta = std::sqrt(1.0 - cos_theta * cos_theta);

        const double phi = two_pi * uniform(rng);

        Vector3 position{
            center.x + r * sin_theta * std::cos(phi),
            center.y + r * sin_theta * std::sin(phi),
            center.z + r * cos_theta
        };

        Vector3 velocity{
            (uniform(rng) - rotational_bias) * dispersion,
            (uniform(rng) - rotational_bias) * dispersion,
            (uniform(rng) - rotational_bias) * dispersion
        };

        particles.push_back( make_particle( position, velocity, masses[i], radius_scale ));
    }

    return particles;
}