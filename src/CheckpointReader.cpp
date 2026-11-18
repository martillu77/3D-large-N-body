#include "CheckpointReader.h"
#include "HDF5Mutex.h"

#include <H5Cpp.h>

#include <filesystem>
#include <vector>
#include <string>
#include <stdexcept>
#include <limits>

namespace {

std::filesystem::path find_latest_checkpoint( const std::string& simulation_name) {
    const std::filesystem::path directory("checkpoints");

    if (!std::filesystem::exists(directory)) throw std::runtime_error("Checkpoint directory does not exist");

    const std::string prefix = simulation_name + "_";

    double latest_time = -std::numeric_limits<double>::infinity();
    std::filesystem::path latest_file;

    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (!entry.is_regular_file()) continue;

        const auto path = entry.path();

        if (path.extension() != ".h5") continue;

        const std::string filename = path.stem().string();

        if (filename.compare(0, prefix.size(), prefix) != 0) continue;

        const std::string time_string = filename.substr(prefix.size());

        try {
            std::size_t pos = 0;
            const double time = std::stod(time_string, &pos);

            if (pos != time_string.size()) continue;

            if (time > latest_time) {
                latest_time = time;
                latest_file = path;
            }
        }
        catch (const std::exception&) {
            continue;
        }
    }

    if (latest_file.empty())
        throw std::runtime_error("No checkpoint found for simulation: " + simulation_name);

    return latest_file;
}

} // namespace


Checkpoint CheckpointReader::load_latest(
    const std::string& simulation_name) {

    const auto filepath = find_latest_checkpoint(simulation_name);

    std::lock_guard<std::mutex> lock(hdf5_mutex());

    H5::H5File file(filepath.string(), H5F_ACC_RDONLY);
    H5::Group group = file.openGroup("/state");

    Checkpoint checkpoint{};

    H5::Attribute step_attr = group.openAttribute("step");
    step_attr.read(H5::PredType::NATIVE_UINT64, &checkpoint.step);

    H5::Attribute time_attr = group.openAttribute("time");
    time_attr.read(H5::PredType::NATIVE_DOUBLE, &checkpoint.time);

    H5::Attribute dt_attr = group.openAttribute("dt");
    dt_attr.read(H5::PredType::NATIVE_DOUBLE, &checkpoint.dt);

    auto read_dataset = [&group](const std::string& name, int expected_rank, std::vector<hsize_t>& dimensions) {
        H5::DataSet dataset = group.openDataSet(name);
        H5::DataSpace space = dataset.getSpace();

        const int rank = space.getSimpleExtentNdims();

        if (rank != expected_rank) throw std::runtime_error("Invalid dimensions in dataset: " + name);

        dimensions.resize(rank);
        space.getSimpleExtentDims(dimensions.data());

        return dataset;
    };

    std::vector<hsize_t> dims;

    auto positions_ds = read_dataset("positions", 2, dims);
    if (dims[1] != 3) throw std::runtime_error("Invalid positions dataset");

    const hsize_t n = dims[0];

    std::vector<double> positions(3 * n);
    std::vector<double> velocities(3 * n);
    std::vector<double> accelerations(3 * n);
    std::vector<double> masses(n);
    std::vector<double> radii(n);

    std::vector<hsize_t> dims_check;

    auto read_vector3 = [&](const std::string& name, std::vector<double>& values) {
        auto dataset = read_dataset(name, 2, dims_check);

        if (dims_check[0] != n || dims_check[1] != 3)
            throw std::runtime_error("Inconsistent dataset: " + name);

        if (!values.empty())
            dataset.read(values.data(), H5::PredType::NATIVE_DOUBLE);
    };

    if (!positions.empty())
        positions_ds.read(positions.data(), H5::PredType::NATIVE_DOUBLE);

    read_vector3("velocities", velocities);
    read_vector3("accelerations", accelerations);

    auto read_scalar = [&](const std::string& name, std::vector<double>& values) {
        auto dataset = read_dataset(name, 1, dims_check);

        if (dims_check[0] != n)
            throw std::runtime_error("Inconsistent dataset: " + name);

        if (!values.empty())
            dataset.read(values.data(), H5::PredType::NATIVE_DOUBLE);
    };

    read_scalar("masses", masses);
    read_scalar("radii", radii);

    checkpoint.particles.resize(n);

    for (std::size_t i = 0; i < n; ++i) {
        auto& p = checkpoint.particles[i];

        p.position.x = positions[3*i];
        p.position.y = positions[3*i + 1];
        p.position.z = positions[3*i + 2];

        p.velocity.x = velocities[3*i];
        p.velocity.y = velocities[3*i + 1];
        p.velocity.z = velocities[3*i + 2];

        p.acceleration.x = accelerations[3*i];
        p.acceleration.y = accelerations[3*i + 1];
        p.acceleration.z = accelerations[3*i + 2];

        p.mass = masses[i];
        p.radius = radii[i];
    }

    return checkpoint;
}