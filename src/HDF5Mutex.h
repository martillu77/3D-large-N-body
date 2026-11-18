#ifndef HDF5_MUTEX_H
#define HDF5_MUTEX_H

#include <mutex>

inline std::mutex& hdf5_mutex()
{
    static std::mutex mutex;
    return mutex;
}

#endif