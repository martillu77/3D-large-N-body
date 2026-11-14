from dataclasses import dataclass
from pathlib import Path

import h5py
import numpy as np


@dataclass
class Snapshot:
    step: int
    time: float
    positions: np.ndarray
    velocities: np.ndarray
    masses: np.ndarray


def load_snapshots(filename: str | Path) -> list[Snapshot]:
    """Load all complete snapshots from an HDF5 simulation file."""

    snapshots = []

    with h5py.File(filename, "r") as h5file:
        if "snapshots" not in h5file:
            raise ValueError("HDF5 file does not contain a 'snapshots' group")

        group = h5file["snapshots"]

        # Sort by numerical step, not lexicographically.
        snapshot_names = sorted( group.keys(), key=lambda name: int(name.removeprefix("step_")) )

        for name in snapshot_names:
            snapshot_group = group[name]

            required = {"positions", "velocities", "masses"}
            if not required.issubset(snapshot_group.keys()):
                continue

            positions = snapshot_group["positions"][:]
            velocities = snapshot_group["velocities"][:]
            masses = snapshot_group["masses"][:]

            n = len(masses)

            if positions.shape != (n, 3) or velocities.shape != (n, 3):
                continue

            snapshots.append(
                Snapshot(
                    step=int(snapshot_group.attrs["step"]),
                    time=float(snapshot_group.attrs["time"]),
                    positions=positions,
                    velocities=velocities,
                    masses=masses,
                )
            )

    if not snapshots:
        raise ValueError("No valid snapshots found in the HDF5 file")

    return snapshots