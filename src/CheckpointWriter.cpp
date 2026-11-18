#include "CheckpointWriter.h"
#include "HDF5Mutex.h"

#include <H5Cpp.h>

#include <filesystem>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <vector>
#include <stdexcept>

void CheckpointWriter::write(const Checkpoint& checkpoint) {
    // Serialize all HDF5 operations across writers:
    std::lock_guard<std::mutex> lock(hdf5_mutex());

    // Create checkpoint directory if necessary:
    std::filesystem::create_directories(directory_);

    // Generate filename using simulation time:
    std::ostringstream filename;
    filename << simulation_name_ << "_"
             << std::setprecision(std::numeric_limits<double>::max_digits10)
             << checkpoint.time << ".h5";

    const auto filepath = std::filesystem::path(directory_) / filename.str();

    // Do not overwrite an existing checkpoint:
    if (std::filesystem::exists(filepath)) {
        throw std::runtime_error( "Checkpoint already exists: " + filepath.string() );
    }

    // Create HDF5 file:
    H5::H5File file(filepath.string(), H5F_ACC_EXCL);

    // Create state group:
    H5::Group state_group = file.createGroup("/state");

    // Write metadata attributes:
    H5::DataSpace scalar_space(H5S_SCALAR);

    auto step_attribute = state_group.createAttribute( "step", H5::PredType::NATIVE_UINT64, scalar_space );
    step_attribute.write( H5::PredType::NATIVE_UINT64, &checkpoint.step );

    auto time_attribute = state_group.createAttribute( "time", H5::PredType::NATIVE_DOUBLE, scalar_space );
    time_attribute.write( H5::PredType::NATIVE_DOUBLE, &checkpoint.time );

    auto dt_attribute = state_group.createAttribute( "dt", H5::PredType::NATIVE_DOUBLE, scalar_space );
    dt_attribute.write( H5::PredType::NATIVE_DOUBLE, &checkpoint.dt );

    // Prepare particle data:
    const hsize_t n = checkpoint.particles.size();

    std::vector<double> positions;
    std::vector<double> velocities;
    std::vector<double> accelerations;
    std::vector<double> masses;
    std::vector<double> radii;

    positions.reserve(3 * n);
    velocities.reserve(3 * n);
    accelerations.reserve(3 * n);
    masses.reserve(n);
    radii.reserve(n);

    for (const auto& p : checkpoint.particles) {

        positions.push_back(p.position.x);
        positions.push_back(p.position.y);
        positions.push_back(p.position.z);

        velocities.push_back(p.velocity.x);
        velocities.push_back(p.velocity.y);
        velocities.push_back(p.velocity.z);

        accelerations.push_back(p.acceleration.x);
        accelerations.push_back(p.acceleration.y);
        accelerations.push_back(p.acceleration.z);

        masses.push_back(p.mass);
        radii.push_back(p.radius);
    }

    // Positions, velocities and accelerations: [N, 3]
    const hsize_t dims_2d[2] = {n, 3};
    H5::DataSpace space_2d(2, dims_2d);

    auto position_dataset = state_group.createDataSet(
        "positions",
        H5::PredType::NATIVE_DOUBLE,
        space_2d
    );

    auto velocity_dataset = state_group.createDataSet(
        "velocities",
        H5::PredType::NATIVE_DOUBLE,
        space_2d
    );

    auto acceleration_dataset = state_group.createDataSet(
        "accelerations",
        H5::PredType::NATIVE_DOUBLE,
        space_2d
    );

    if (n > 0) {
        position_dataset.write( positions.data(), H5::PredType::NATIVE_DOUBLE );

        velocity_dataset.write( velocities.data(), H5::PredType::NATIVE_DOUBLE );

        acceleration_dataset.write( accelerations.data(), H5::PredType::NATIVE_DOUBLE );
    }

    // Masses and radii: [N]
    const hsize_t dims_1d[1] = {n};
    H5::DataSpace space_1d(1, dims_1d);

    auto mass_dataset = state_group.createDataSet(
        "masses",
        H5::PredType::NATIVE_DOUBLE,
        space_1d
    );

    auto radius_dataset = state_group.createDataSet(
        "radii",
        H5::PredType::NATIVE_DOUBLE,
        space_1d
    );

    if (n > 0) {
        mass_dataset.write( masses.data(), H5::PredType::NATIVE_DOUBLE );

        radius_dataset.write( radii.data(), H5::PredType::NATIVE_DOUBLE );
    }

    file.flush(H5F_SCOPE_GLOBAL);
}