#ifndef SNAPSHOT_WRITER_H
#define SNAPSHOT_WRITER_H

#include "Writer.h"
#include "SimulationState.h"

class SnapshotWriter : public Writer<Snapshot> {

public:
    explicit SnapshotWriter(std::size_t queue_capacity)
        : Writer<Snapshot>(queue_capacity) {}

    ~SnapshotWriter() override {
        stop_noexcept();
    }

    void set_filename(const std::string& filename) {
        filename_ = filename;
    }

protected:
    void write(const Snapshot& snapshot) override;


private:
    std::string filename_ = "simulation.h5";
    
};

#endif