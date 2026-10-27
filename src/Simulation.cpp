#include "Simulation.h"
#include "SimpleVerletEngine.h"


#include <chrono>
#include <iomanip>
#include <sstream>

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

    simulation_thread_ = std::thread(&Simulation::run, this); // Start the simulation in a separate thread
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

    running_ = false;
    paused_ = false;
}

void Simulation::run() {
    
    data_.eta_dt = configuration_.eta_dt();
    data_.particles = InitialConditions::generate(configuration_, simulation_units_);
    
    engine_->initialize(data_);

    while (!stop_requested_) {
        if (paused_) {
            std::this_thread::sleep_for( std::chrono::milliseconds(100) );
            continue;
        }

        engine_->step(data_);

        ++step_;
        data_.time += data_.dt;

        std::this_thread::sleep_for( std::chrono::milliseconds(1) );
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