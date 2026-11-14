#include "Simulation.h"
#include "SimpleVerletEngine.h"


#include <chrono>
#include <iomanip>
#include <sstream>

#include <fstream>
#include <limits>
#include <stdexcept>
#include <cmath>

Simulation::Simulation()
    : running_(false),
      paused_(false),
      stop_requested_(false),
      step_(0),
      simulation_time_(0.0)
{

}

Simulation::~Simulation() {
    stop();
}

void Simulation::load_configuration(const Configuration& configuration) {
    if (running_) return;
    configuration_ = configuration;

    if (configuration_.model() == "SimpleVerlet") {
        engine_ = std::make_unique<SimpleVerletEngine>();

    } else {
        throw std::runtime_error("Unknown simulation model: " + configuration_.model());
    }

}

void Simulation::start() {
    if (running_) return;

    stop_requested_ = false;
    paused_ = false;
    running_ = true;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    simulation_thread_ = std::thread(&Simulation::run, this); // Start the simulation in a separate thread
    ///////////////////////////////////////////////////////////////////////////////////////////////////////

    snapshot_writer_.set_filename( configuration_.simulation_name() + ".h5" );
    //checkpoint_writer_.set_filename( configuration_.simulation_name() + "_chkpt.h5" );

    snapshot_writer_.start();
    checkpoint_writer_.start();
}

void Simulation::pause() {
    if (running_) paused_ = true;
}

void Simulation::resume() {
    if (running_ && paused_) paused_ = false;
}

void Simulation::stop() {
    //if (!running_) return;

    stop_requested_ = true;

    if (simulation_thread_.joinable()) simulation_thread_.join();

    snapshot_writer_.stop();
    checkpoint_writer_.stop();

    running_ = false;
    paused_ = false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////
void Simulation::run() {
    
    data_.eta_dt = configuration_.eta_dt();
    data_.particles = InitialConditions::generate(configuration_, simulation_units_);
    
    engine_->initialize(data_);
    write_header();
    double active_fraction = snapshot_dist_fraction_.load();
    next_snapshot_time_ = data_.time + engine_->estimate_snapshot_interval(data_, active_fraction);

    while (!stop_requested_) {
        if (paused_) {
            std::this_thread::sleep_for( std::chrono::milliseconds(100) );
            continue;
        }

        engine_->step(data_);

        ++step_;
        data_.time += data_.dt;

        double requested_fraction = snapshot_dist_fraction_.load();
        if (data_.time >= next_snapshot_time_ || requested_fraction != active_fraction) {

            if (data_.time >= next_snapshot_time_) snapshot_writer_.enqueue(create_snapshot());

            active_fraction = requested_fraction;
            next_snapshot_time_ = data_.time + engine_->estimate_snapshot_interval(data_, active_fraction);

        }
    }

    running_ = false;
}

std::string Simulation::status() const {
    if (!running_)
        return "STOPPED";

    if (paused_)
        return "PAUSED";

    return "RUNNING";
}

void Simulation::set_snapshot_dist_fraction(double value) {
    if (!std::isfinite(value) || value <= 0.0) {
        throw std::invalid_argument( "snapshot_dist_fraction must be positive and finite" );
    }

    snapshot_dist_fraction_.store(value);
}





Snapshot Simulation::create_snapshot() const {
    Snapshot snapshot;

    snapshot.step = step_;
    snapshot.time = data_.time;
    snapshot.particles = data_.particles;

    return snapshot;
}

void Simulation::write_header() const {

    const std::string filename = configuration_.simulation_name() + ".json";
    std::ofstream out(filename);

    if (!out) throw std::runtime_error("Cannot create simulation header: " + filename);

    out << std::setprecision(std::numeric_limits<double>::max_digits10);

    out << "{\n";

    out << "  \"simulation_name\": \"" << configuration_.simulation_name() << "\",\n";

    out << "  \"integrator\": \"" << configuration_.model() << "\",\n";

    out << "  \"initial_conditions\": \"" << configuration_.initial_conditions() << "\",\n";

    out << "  \"initial_timestep\": " << data_.dt << ",\n";
    out << "  \"eta_dt\": " << data_.eta_dt << ",\n";
    out << "  \"snapshot_dist_fraction\": " << configuration_.snapshot_dist_fraction() << ",\n";

    out << "  \"units\": {\n";
    out << "    \"mass_scale\": " << simulation_units_.mass_scale << ",\n";
    out << "    \"length_scale\": " << simulation_units_.length_scale << ",\n";
    out << "    \"time_scale\": " << simulation_units_.time_scale << "\n";
    out << "  },\n";

    out << "  \"parameters\": {\n";
    out << "    \"particles\": " << configuration_.particles() << ",\n";
    out << "    \"mass\": " << configuration_.mass() << ",\n";
    out << "    \"radius\": " << configuration_.radius() << ",\n";
    out << "    \"distribution_radius\": " << configuration_.distribution_radius() << ",\n";
    out << "    \"stellar_mass\": " << configuration_.stellar_mass() << ",\n";
    out << "    \"cloud_mass\": " << configuration_.cloud_mass() << ",\n";
    out << "    \"disk_mass_fraction\": " << configuration_.disk_mass_fraction() << ",\n";
    out << "    \"lognormal_sigma\": " << configuration_.lognormal_sigma() << ",\n";
    out << "    \"omega\": " << configuration_.omega() << ",\n";
    out << "    \"rotational_bias\": " << configuration_.rotational_bias() << ",\n";
    out << "    \"sigma\": " << configuration_.sigma() << ",\n";
    out << "    \"collisions\": " << (configuration_.collisions() ? "true" : "false") << ",\n";
    out << "    \"restitution\": " << configuration_.restitution() << ",\n";
    out << "    \"seed_provided\": " << (configuration_.seed_provided() ? "true" : "false") << ",\n";
    out << "    \"seed\": " << configuration_.seed() << "\n";
    out << "  }\n";

    out << "}\n";

    if (!out) throw std::runtime_error("Error writing simulation header: " + filename);

}



std::string Simulation::dump() const {
    std::ostringstream out;

    out << "Simulation\n";
    out << "----------\n";
    out << "State: " << status() << '\n';
    out << "Step:  " << step_.load() << '\n';
    out << "Time:  "
        << std::fixed << std::setprecision(6)
        << simulation_time_.load() << '\n';

    out << "\nConfiguration\n";
    out << "-------------\n";
    out << "Particles:                  " << configuration_.particles() << '\n';
    out << "Initial conditions:         " << configuration_.initial_conditions() << '\n';
    out << "Particle mass:              " << configuration_.mass() << '\n';
    out << "Particle radius:            " << configuration_.radius() << '\n';

    out << "\nInitial conditions parameters:\n";
    out << "Initial distribution model: " << configuration_.initial_conditions() << '\n';
    out << "Distribution radius:        " << configuration_.distribution_radius() << '\n';
    out << "Stellar mass:               " << configuration_.stellar_mass() << '\n';
    out << "Cloud mass:                 " << configuration_.cloud_mass() << '\n';

    out << "\nRotation parameters:\n";
    out << "Omega:                      " << configuration_.omega() << '\n';
    out << "Rotational bias:            " << configuration_.rotational_bias() << '\n';
    out << "Sigma:                      " << configuration_.sigma() << '\n';

    out << "\nCollision parameters:\n";
    out << "Collisions:                 " << (configuration_.collisions() ? "enabled" : "disabled") << '\n';
    if (configuration_.collisions()) 
        out << "Restitution:            " << configuration_.restitution() << '\n';

    out << "\nSimulation parameters:\n";
    out << "eta_dt:                     " << configuration_.eta_dt() << '\n';
    out << "Model:                      " << configuration_.model() << '\n';
    if (configuration_.seed_provided())
    out << "Random seed:                " << configuration_.seed() << '\n';
    else 
    out << "Random seed:                (not provided)\n";

    return out.str();
}