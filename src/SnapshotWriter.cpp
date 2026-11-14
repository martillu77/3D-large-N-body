#include "SnapshotWriter.h"

#include <H5Cpp.h>

#include <filesystem>
#include <iomanip>
#include <memory>
#include <sstream>
#include <vector>

void SnapshotWriter::write(const Snapshot& snapshot) {


    // Open or create the HDF5 file:
    std::unique_ptr<H5::H5File> file;

    if (std::filesystem::exists(filename_)) {
        file = std::make_unique<H5::H5File>( filename_, H5F_ACC_RDWR );
    } else {
        file = std::make_unique<H5::H5File>( filename_, H5F_ACC_EXCL );
    }

    // Open or create the snapshots group:
    H5::Group snapshots_group;

    if (H5Lexists(file->getId(), "/snapshots", H5P_DEFAULT) > 0) {
        snapshots_group = file->openGroup("/snapshots");
    } else {
        snapshots_group = file->createGroup("/snapshots");
    }

    // Create a group for this snapshot:
    std::ostringstream group_name;
    group_name << "step_"
               << std::setw(12) << std::setfill('0')
               << snapshot.step;

    H5::Group group = snapshots_group.createGroup(group_name.str());

    // Write metadata attributes:
    H5::DataSpace scalar_space(H5S_SCALAR);

    auto step_attribute = group.createAttribute( "step", H5::PredType::NATIVE_UINT64, scalar_space );

    step_attribute.write( H5::PredType::NATIVE_UINT64, &snapshot.step );

    auto time_attribute = group.createAttribute( "time", H5::PredType::NATIVE_DOUBLE, scalar_space );

    time_attribute.write( H5::PredType::NATIVE_DOUBLE, &snapshot.time );

    // Prepare particle data:
    const hsize_t n = snapshot.particles.size();

    std::vector<double> positions;
    std::vector<double> velocities;
    std::vector<double> masses;

    positions.reserve(3 * n);
    velocities.reserve(3 * n);
    masses.reserve(n);

    for (const auto& p : snapshot.particles) {
        positions.push_back(p.position.x);
        positions.push_back(p.position.y);
        positions.push_back(p.position.z);

        velocities.push_back(p.velocity.x);
        velocities.push_back(p.velocity.y);
        velocities.push_back(p.velocity.z);

        masses.push_back(p.mass);
    }

    // Positions and velocities: [N, 3].
    const hsize_t dims_2d[2] = {n, 3};
    H5::DataSpace space_2d(2, dims_2d);

    auto position_dataset = group.createDataSet(
        "positions",
        H5::PredType::NATIVE_DOUBLE,
        space_2d);

    auto velocity_dataset = group.createDataSet(
        "velocities",
        H5::PredType::NATIVE_DOUBLE,
        space_2d);

    if (n > 0) {
        position_dataset.write(positions.data(), H5::PredType::NATIVE_DOUBLE);
        velocity_dataset.write(velocities.data(), H5::PredType::NATIVE_DOUBLE);
    }

    // Masses: [N].
    const hsize_t dims_1d[1] = {n};
    H5::DataSpace space_1d(1, dims_1d);

    auto mass_dataset = group.createDataSet(
        "masses",
        H5::PredType::NATIVE_DOUBLE,
        space_1d);

    if (n > 0) {
        mass_dataset.write( masses.data(), H5::PredType::NATIVE_DOUBLE );
    }

    file->flush(H5F_SCOPE_GLOBAL);
}