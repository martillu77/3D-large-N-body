#ifndef CHECKPOINT_WRITER_H
#define CHECKPOINT_WRITER_H

#include "Writer.h"
#include "SimulationState.h"

#include <string>

class CheckpointWriter : public Writer<Checkpoint> {

public:
    explicit CheckpointWriter(std::size_t queue_capacity)
        : Writer<Checkpoint>(queue_capacity) {}

    ~CheckpointWriter() override {
        stop_noexcept();
    }

    void set_filename(const std::string& name) {
        simulation_name_ = name;
    }

protected:
    void write(const Checkpoint& checkpoint) override;

private:
    std::string simulation_name_ = "unnamed";
    std::string directory_ = "checkpoints";
};

#endif