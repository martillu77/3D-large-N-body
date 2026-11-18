#ifndef SIMULATION_H
#define SIMULATION_H

#include "Configuration.h"
#include "Engine.h"
#include "SimulationData.h"
#include "InitialConditions.h"

#include "SimulationState.h"

#include "SnapshotWriter.h"
#include "CheckpointWriter.h"
#include "CheckpointReader.h"

#include <atomic>
#include <cstdint>
#include <string>
#include <thread>
#include <memory>  // std::unique_ptr

class Simulation {
public:
    Simulation();
    ~Simulation();

    void load_configuration(const Configuration& configuration);
    void load_checkpoint(const std::string& name);

    void start();
    void pause();
    void resume();
    void stop();

    void set_snapshot_dist_fraction(double value);

    std::string status() const;
    std::string dump() const;
    void request_checkpoint();

private:
    void run();
    void write_header() const;

    SnapshotWriter snapshot_writer_{8};
    CheckpointWriter checkpoint_writer_{2};

    std::atomic<bool> running_;
    std::atomic<bool> paused_;
    std::atomic<bool> stop_requested_;

    std::atomic<std::uint64_t> step_;
    std::atomic<double> simulation_time_;

    std::atomic<double> snapshot_dist_fraction_{0.01};

    std::thread simulation_thread_;

    Configuration configuration_;
    SimulationUnits simulation_units_;

    SimulationData data_;                  // particle (pos, vel, acc, mass, radius), dt, eta_dt, time
    std::unique_ptr<Engine> engine_;

    double next_snapshot_time_ = 0.0;
    std::uint64_t snapshot_interval_steps_ = 1000;
    Snapshot create_snapshot() const;

    std::atomic<bool> checkpoint_requested_{false};

    Checkpoint create_checkpoint() const;
    void process_checkpoint_request();

    bool checkpoint_loaded_ = false;

};

#endif