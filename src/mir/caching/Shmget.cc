// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/caching/Shmget.h"

#include <sys/ipc.h>

#include <sstream>

#include "eckit/config/Resource.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/memory/Shmget.h"
#include "eckit/os/Stat.h"

#include "mir/config/LibMir.h"
#include "mir/util/Error.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::caching {


key_t Shmget::calculate_key(const eckit::PathName& path) {
    const auto name = path.realName().asString();

    // Use time of creation epoch as proj_id for ftok to add 8 more bits of entropy
    eckit::Stat::Struct s;
    SYSCALL(eckit::Stat::stat(name.c_str(), &s));

    const auto proj_id = static_cast<int>(
#if defined(HAS_SYS_STAT_ST_CTIMESPEC)
        s.st_ctimespec.tv_sec
#else
        s.st_ctim.tv_sec
#endif
    );

    if (key_t key = ::ftok(name.c_str(), proj_id); key != -1) {
        return key;
    }

    std::ostringstream ss;
    ss << "Shmget::calculate_key: ::ftok(" << name << "), " << util::Error();
    const auto msg = ss.str();

    Log::error() << msg << std::endl;
    throw exception::FailedSystemCall(msg);
}


int Shmget::shmget(key_t key, size_t size) {
    constexpr int shmflg = IPC_CREAT | 0600;

#ifdef SHM_HUGETLB
    // Prefer huge pages when the system has them reserved (SHM_HUGETLB): fewer page-table
    // entries and fewer page faults for the many processes attaching to this same segment.
    // The kernel rounds shmsize up to the huge-page boundary itself; fall back to regular
    // pages if huge pages aren't available/reserved on this node.
    static const bool hugetlb =
        eckit::LibResource<bool, LibMir>("mir-shared-memory-loader-hugepages;$MIR_CACHE_SHM_HUGETLB", true);
    const int shmid = shmget(key, size, hugetlb ? (shmflg | SHM_HUGETLB) : shmflg);
#else
    const int shmid = shmget(key, size, shmflg);
#endif

#ifdef SHM_PAGESIZE
    if (shmid >= 0) {
        // back the shared memory with 64K pages, if possible (failing is not an error)
        shmid_ds buf{};
        buf.shm_pagesize = 64 * 1024;
        shmctl(shmid, SHM_PAGESIZE, &buf);
    }
#endif

    return shmid;
}


int Shmget::shmget(key_t key, size_t size, int shmflg) {
    int shmid = eckit::Shmget::shmget(key, size, shmflg);
#ifdef SHM_HUGETLB
    if (shmid < 0 && (shmflg & SHM_HUGETLB) != 0) {
        // no huge pages reserved on this node, fall back to regular pages
        shmid = eckit::Shmget::shmget(key, size, shmflg & ~SHM_HUGETLB);
    }
#endif
    return shmid;
}


void* Shmget::shmat(int shmid, const void* shmaddr, int shmflg) {
    return eckit::Shmget::shmat(shmid, shmaddr, shmflg);
}


int Shmget::shmdt(const void* shmaddr) {
    return eckit::Shmget::shmdt(shmaddr);
}


int Shmget::shmctl(int shmid, int cmd, shmid_ds* buf) {
    // FIXME: add to eckit::Shmget interface
    return ::shmctl(shmid, cmd, buf);
}


size_t Shmget::max_size() {
#ifdef IPC_INFO
    shminfo info{};
    SYSCALL(shmctl(0, IPC_INFO, reinterpret_cast<shmid_ds*>(&info)));
    return static_cast<size_t>(info.shmmax >> 10) * 1024;
#else
    return 0;
#endif
}


}  // namespace mir::caching
