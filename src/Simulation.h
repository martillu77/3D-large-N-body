#ifndef SIMULATION_H
#define SIMULATION_H

#include "Configuration.h"
#include "Engine.h"
#include "SimulationData.h"
#include "InitialConditions.h"


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

    void start();
    void pause();
    void resume();
    void stop();

    std::string status() const;
    std::string dump() const;

private:
    void run();

    std::atomic<bool> running_;
    std::atomic<bool> paused_;
    std::atomic<bool> stop_requested_;

    std::atomic<std::uint64_t> step_;
    std::atomic<double> simulation_time_;

    std::thread simulation_thread_;

    Configuration configuration_;
    SimulationUnits simulation_units_;

    SimulationData data_;
    std::unique_ptr<Engine> engine_;
};

#endif