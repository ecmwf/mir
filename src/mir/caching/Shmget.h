// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <sys/shm.h>  // shmid_ds is a macro on macOS, it can't be forward declared

#include <cstddef>


namespace eckit {
class PathName;
}


namespace mir::caching {


/// System V shared memory, as eckit::Shmget, with MIR's defaults
class Shmget {
public:
    /// Calculate the key of a file, by ftok with its creation time (more entropy)
    static key_t calculate_key(const eckit::PathName&);

    /// Get or create (0600) a segment, on huge pages unless disabled ($MIR_CACHE_SHM_HUGETLB)
    static int shmget(key_t, size_t size);

    /// Get a segment
    static int shmget(key_t, size_t size, int shmflg);

    /// Attach a segment
    static void* shmat(int shmid, const void* shmaddr, int shmflg);

    /// Detach a segment
    static int shmdt(const void* shmaddr);

    /// Control a segment
    static int shmctl(int shmid, int cmd, shmid_ds* buf);

    /// Get the maximum segment size
    static size_t max_size();
};


}  // namespace mir::caching
