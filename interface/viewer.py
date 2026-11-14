#sudo apt install python3-numpy python3-matplotlib
#python3 -m pip install h5py

import argparse

from data_reader import load_snapshots
from renderer import Renderer


def main():
    parser = argparse.ArgumentParser(
        description="3D N-body simulation viewer"
    )

    parser.add_argument(
        "file",
        help="HDF5 simulation file"
    )

    args = parser.parse_args()

    snapshots = load_snapshots(args.file)

    print(f"Loaded {len(snapshots)} snapshots")
    print(f"Steps: {snapshots[0].step} - {snapshots[-1].step}")
    print(f"Time: {snapshots[0].time} - {snapshots[-1].time}")

    renderer = Renderer(snapshots)
    renderer.show()


if __name__ == "__main__":
    main()