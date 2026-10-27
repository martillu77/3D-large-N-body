#ifndef SIMPLE_VERLET_ENGINE_H
#define SIMPLE_VERLET_ENGINE_H

#include "Engine.h"
#include "SimulationData.h"

// Simple Verlet integration engine
//    overrides the pure virtual functions of the Engine base class
class SimpleVerletEngine : public Engine {

public:
    SimpleVerletEngine();
    ~SimpleVerletEngine() override = default;

    void initialize(SimulationData& data) override;
    void step(SimulationData& data) override;
    void reset(SimulationData& data) override;

};

#endif