#ifndef ENGINE_H
#define ENGINE_H

#include "SimulationData.h"

// Base class for simulation engines (remember:  =0 means pure virtual functions)
class Engine {
public:
    virtual ~Engine() = default;

    virtual void initialize(SimulationData& data) = 0;
    virtual void step(SimulationData& data) = 0;
    virtual void reset(SimulationData& data) = 0;

    virtual double estimate_snapshot_interval(const SimulationData& data, double snapshot_dist_fraction) const = 0;
};

#endif