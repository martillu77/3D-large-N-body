#include "Configuration.h"

#include <fstream>
#include <sstream>
#include <stdexcept>

#include <nlohmann/json.hpp>

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
Configuration::Configuration()
    : particles_(0),
      mass_(1.0),
      initial_conditions_("rotating_disk"),
      radius_(0.01),
      distrib_radius_(10.0),

      stellar_mass_(1.0),
      cloud_mass_(1.0),

      disk_mass_fraction_(0.002),
      lognormal_sigma_(1.5),

      omega_(1.0),
      rotational_bias_(0.5),
      sigma_(1.5),

      collisions_(false),
      restitution_(1.0),

      eta_dt_(0.01),
      model_("SimpleVerlet"),
      seed_(0),
      seed_provided_(false),

      snapshot_dist_fraction_(0.01)
{
}

/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////

bool Configuration::load(const std::string& filename) {
    std::ifstream file(filename);

    if (!file) return false;

    std::string line;

    try {
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            std::istringstream input(line);

            std::string key;
            std::string value;

            if (!(input >> key >> value))       continue;
            if (key[0] == '#')                  continue;

            if      (key == "simulation_name")    simulation_name_ = value;
            else if (key == "particles")          particles_ = std::stoi(value);          // Number of particles
            else if (key == "mass")               mass_ = std::stod(value);               // Particle mass
            else if (key == "radius")             radius_ = std::stod(value);             // Particle radius

            else if (key == "initial_conditions") initial_conditions_ = value;            // Initial conditions model
            else if (key == "distrib_radius")     distrib_radius_ = std::stod(value);     // Initial distribution radius
            else if (key == "stellar_mass")       stellar_mass_ = std::stod(value);
            else if (key == "cloud_mass")         cloud_mass_ = std::stod(value);

            else if (key == "disk_mass_fraction") disk_mass_fraction_ = std::stod(value); // Fraction of stellar mass in the disk                             (for proto-disc)
            else if (key == "lognormal_sigma")    lognormal_sigma_ = std::stod(value);    // Standard deviation of log-normal distribution of particle masses (for proto-disc)

            else if (key == "omega")              omega_ = std::stod(value);
            else if (key == "rotational_bias")    rotational_bias_ = std::stod(value);
            else if (key == "sigma")              sigma_ = std::stod(value);

            else if (key == "collisions")         collisions_ = std::stoi(value);         // Are collisions enabled (1) or not (0)
            else if (key == "restitution")        restitution_ = std::stod(value);        //     If enabled: restitution coefficient for collisions (1 = perfectly elastic, 0 = perfectly inelastic)

            else if (key == "eta_dt")             eta_dt_ = std::stod(value);             // Safety coeficient when choosing the simulation's time step: 
                                                                                          //   ->   time_step = eta_dt * min_dynamical_time (SimpleVerletEngine)
            else if (key == "model")              model_ = value;                         // Simulation model (e.g. "SimpleVerlet")
            else if (key == "seed") {
                seed_ = std::stoull(value);                                               // Random seed for reproducibility 
                if (seed_ == 0)                   seed_provided_ = false;
                else                              seed_provided_ = true; 
            }

            else if (key == "omega")              omega_ = std::stod(value);
            else if (key == "rotational_bias")    rotational_bias_ = std::stod(value);
            else if (key == "sigma")              sigma_ = std::stod(value);

            else if (key == "collisions")         collisions_ = std::stoi(value);         // Are collisions enabled (1) or not (0)
            else if (key == "restitution")        restitution_ = std::stod(value);        //     If enabled: restitution coefficient for collisions (1 = perfectly elastic, 0 = perfectly inelastic)

            else if (key == "seed") {
                seed_ = std::stoull(value);                                               // Random seed for reproducibility 
                if (seed_ == 0)                   seed_provided_ = false;
                else                              seed_provided_ = true; 
            }

            else if (key == "snapshot_dist_fraction") snapshot_dist_fraction_ = std::stod(value);


        }
    }
    catch (const std::exception&) {
        return false;
    }

    return true;
}

/////////////////////////////////////////////////////////////////

bool Configuration::load_json(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) return false;

    try {
        using json = nlohmann::json;

        json j;
        file >> j;

        Configuration candidate_config;
        const auto& p = j.at("parameters");

        candidate_config.simulation_name_ = j.at("simulation_name").get<std::string>();
        candidate_config.model_ = j.at("model").get<std::string>();
        candidate_config.initial_conditions_ = j.at("initial_conditions").get<std::string>();

        candidate_config.eta_dt_ = j.at("eta_dt").get<double>();
        candidate_config.snapshot_dist_fraction_ = j.at("snapshot_dist_fraction").get<double>();

        candidate_config.particles_ = p.at("particles").get<int>();
        candidate_config.mass_ = p.at("mass").get<double>();
        candidate_config.radius_ = p.at("radius").get<double>();
        candidate_config.distrib_radius_ = p.at("distribution_radius").get<double>();

        candidate_config.stellar_mass_ = p.at("stellar_mass").get<double>();
        candidate_config.cloud_mass_ = p.at("cloud_mass").get<double>();

        candidate_config.disk_mass_fraction_ = p.at("disk_mass_fraction").get<double>();
        candidate_config.lognormal_sigma_ = p.at("lognormal_sigma").get<double>();

        candidate_config.omega_ = p.at("omega").get<double>();
        candidate_config.rotational_bias_ = p.at("rotational_bias").get<double>();
        candidate_config.sigma_ = p.at("sigma").get<double>();

        candidate_config.collisions_ = p.at("collisions").get<bool>();
        candidate_config.restitution_ = p.at("restitution").get<double>();

        candidate_config.seed_provided_ = p.at("seed_provided").get<bool>();
        candidate_config.seed_ = p.at("seed").get<std::uint64_t>();

        *this = candidate_config;
    }
    catch (const std::exception&) {
        return false;
    }

    return true;
}


/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////






int Configuration::particles() const {
    return particles_;
}
double Configuration::mass() const {
    return mass_;
}
double Configuration::radius() const {
    return radius_;
}



const std::string& Configuration::initial_conditions() const {
    return initial_conditions_;
}
double Configuration::distribution_radius() const {
    return distrib_radius_;
}
double Configuration::stellar_mass() const {
    return stellar_mass_;
}
double Configuration::cloud_mass() const {
    return cloud_mass_;
}

double Configuration::disk_mass_fraction() const {   // for proton-disc only
    return disk_mass_fraction_;
}
double Configuration::lognormal_sigma() const {      // for proton-disc only
    return lognormal_sigma_;
}

double Configuration::omega() const {
    return omega_;
}
double Configuration::rotational_bias() const {
    return rotational_bias_;
}
double Configuration::sigma() const {
    return sigma_;
}



bool Configuration::collisions() const {
    return collisions_;
}
double Configuration::restitution() const {
    return restitution_;
}




double Configuration::eta_dt() const {
    return eta_dt_;
}
const std::string& Configuration::model() const {
    return model_;
}
std::uint64_t Configuration::seed() const {
    return seed_;
}
bool Configuration::seed_provided() const {
    return seed_provided_;
}