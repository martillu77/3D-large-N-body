#ifndef CHECKPOINT_WRITER_H
#define CHECKPOINT_WRITER_H

#include "Writer.h"
#include "SimulationState.h"

class CheckpointWriter : public Writer<Checkpoint> {
public:
    explicit CheckpointWriter(std::size_t queue_capacity)
        : Writer<Checkpoint>(queue_capacity) {}

    ~CheckpointWriter() override {
        stop_noexcept();
    }

protected:
    void write(const Checkpoint& checkpoint) override;
};

#endif