#ifndef CHECKPOINT_READER_H
#define CHECKPOINT_READER_H

#include "SimulationState.h"

#include <string>

class CheckpointReader {
public:
    static Checkpoint load_latest(const std::string& simulation_name);
};

#endif