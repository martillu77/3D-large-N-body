#include "Simulation.h"
#include "SimpleVerletEngine.h"


#include <chrono>
#include <iomanip>
#include <sstream>

#include <fstream>
#include <limits>
#include <stdexcept>
#include <cmath>

#include <filesystem>
#include <nlohmann/json.hpp>

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


/////////////////////////////////////////////////////////////////////////////////
void Simulation::load_configuration(const Configuration& configuration) {
    if (running_) return;
    configuration_ = configuration;

    if (configuration_.model() == "SimpleVerlet") {
        engine_ = std::make_unique<SimpleVerletEngine>();

    } else {
        throw std::runtime_error("Unknown simulation model: " + configuration_.model());
    }

}


/////////////////////////////////////////////////////////////////////////////////
void Simulation::load_checkpoint(const std::string& name) {
    if (running_) {
        throw std::runtime_error("Cannot load checkpoint while simulation is running");
    }

    // Load configuration from JSON:
    Configuration candidate_config;
    if (!candidate_config.load_json(name + ".json")) {
        throw std::runtime_error("Cannot load configuration: " + name + ".json");
    }

    // Read JSON metadata, including simulation units:
    std::ifstream file(name + ".json");
    if (!file) {
        throw std::runtime_error("Cannot open simulation metadata: " + name + ".json");
    }

    nlohmann::json j;
    try {
        file >> j;
    } catch (const std::exception& e) {
        throw std::runtime_error( std::string("Invalid simulation metadata: ") + e.what() );
    }

    SimulationUnits candidate_units;
    try {
        const auto& units = j.at("units");
        candidate_units.mass_scale = units.at("mass_scale").get<double>();
        candidate_units.length_scale = units.at("length_scale").get<double>();
        candidate_units.time_scale = units.at("time_scale").get<double>();
    } catch (const std::exception& e) {
        throw std::runtime_error( std::string("Invalid simulation units: ") + e.what() );
    }

    // Load latest checkpoint:
    Checkpoint candidate_checkpoint = CheckpointReader::load_latest(name);

    // Validate checkpoint:
    if (candidate_checkpoint.particles.empty()) {
        throw std::runtime_error("Checkpoint contains no particles");
    }

    // Select simulation engine:
    std::unique_ptr<Engine> candidate_engine;

    if (candidate_config.model() == "SimpleVerlet") {
        candidate_engine = std::make_unique<SimpleVerletEngine>();
    } else {
        throw std::runtime_error( "Unknown simulation model: " + candidate_config.model() );
    }

    // Commit restored state only after everything has loaded successfully:
    configuration_ = candidate_config;
    simulation_units_ = candidate_units;
    engine_ = std::move(candidate_engine);

    data_.particles = std::move(candidate_checkpoint.particles);
    data_.time = candidate_checkpoint.time;
    data_.dt = candidate_checkpoint.dt;
    data_.eta_dt = configuration_.eta_dt();

    step_ = candidate_checkpoint.step;
    simulation_time_ = candidate_checkpoint.time;

    snapshot_dist_fraction_ = configuration_.snapshot_dist_fraction();

    checkpoint_loaded_ = true;
}


/////////////////////////////////////////////////////////////////////////////////
void Simulation::start() {
    if (running_) return;

    const std::string name = configuration_.simulation_name();

    // New simulation: make sure output files do not already exist:
    if (!checkpoint_loaded_) {
        if (std::filesystem::exists(name + ".h5") || std::filesystem::exists(name + ".json")) {
            throw std::runtime_error( "Simulation output already exists: " + name );
        }
    }// else {
        // Resuming simulation: the original output file must already exist:
     //   if (!std::filesystem::exists(name + ".h5")) {
     //       throw std::runtime_error( "Snapshot file not found: " + name + ".h5" );
     //   }
    //}


    stop_requested_ = false;
    paused_ = false;
    running_ = true;

    snapshot_writer_.set_filename( name + ".h5" );
    checkpoint_writer_.set_filename( name );

    snapshot_writer_.start();
    checkpoint_writer_.start();

    ///////////////////////////////////////////////////////////////////////////////////////////////////////
    simulation_thread_ = std::thread(&Simulation::run, this); // Start the simulation in a separate thread
    ///////////////////////////////////////////////////////////////////////////////////////////////////////


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
    
    if (!checkpoint_loaded_) {
        data_.eta_dt = configuration_.eta_dt();
        data_.particles = InitialConditions::generate(configuration_, simulation_units_);
    
        engine_->initialize(data_);
        
        step_ = 0;
        simulation_time_ = data_.time;

        write_header();

    } else {
        // Continue from the restored checkpoint.
        // Write the restored state as the first snapshot of this session.
        snapshot_writer_.enqueue(create_snapshot());
    }


    double active_fraction = snapshot_dist_fraction_.load();
    next_snapshot_time_ = data_.time + engine_->estimate_snapshot_interval(data_, active_fraction);

    while (!stop_requested_) {
        if (paused_) {
            process_checkpoint_request();
            std::this_thread::sleep_for( std::chrono::milliseconds(100) );
            continue;
        }

        engine_->step(data_);
        ++step_;
        data_.time += data_.dt;
        simulation_time_ = data_.time;

        process_checkpoint_request();
//        #if DEBUG_FLAG
//        if (step_ % snapshot_interval_steps_ == 0) {
//            snapshot_writer_.enqueue(create_snapshot());
//        }
//      #endif
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






/////////////////////////////////////////////////////////////////////////////////////////////
void Simulation::request_checkpoint() {
    if (!running_) {
        throw std::runtime_error("Cannot request checkpoint: simulation is not running");
    }

    checkpoint_requested_ = true;
}


Checkpoint Simulation::create_checkpoint() const {
    Checkpoint checkpoint;

    checkpoint.step = step_.load();
    checkpoint.time = data_.time;
    checkpoint.dt = data_.dt;
    checkpoint.particles = data_.particles;

    return checkpoint;
}

void Simulation::process_checkpoint_request() {
    if (!checkpoint_requested_.exchange(false)) return;
    
    snapshot_writer_.enqueue(create_snapshot());
    checkpoint_writer_.enqueue(create_checkpoint());
    
}
/////////////////////////////////////////////////////////////////////////////////////////////








/////////////////////////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////////////////////////










void Simulation::write_header() const {

    const std::string filename = configuration_.simulation_name() + ".json";
    std::ofstream out(filename);

    if (!out) throw std::runtime_error("Cannot create simulation header: " + filename);

    out << std::setprecision(std::numeric_limits<double>::max_digits10);

    out << "{\n";

    out << "  \"simulation_name\": \"" << configuration_.simulation_name() << "\",\n";

    out << "  \"model\": \"" << configuration_.model() << "\",\n";

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