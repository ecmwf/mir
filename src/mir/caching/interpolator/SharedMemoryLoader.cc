/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @author Tiago Quintino
///
/// @date Oct 2016

#include "mir/caching/interpolator/SharedMemoryLoader.h"

#include <errno.h>
#include <unistd.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/sem.h>

#include "eckit/eckit.h"
#include "eckit/os/SemLocker.h"

#include "eckit/memory/Padded.h"
#include "eckit/log/Bytes.h"
#include "eckit/log/BigNum.h"
#include "eckit/config/Resource.h"
#include "eckit/maths/Functions.h"

#include "eckit/log/Timer.h"
#include "eckit/io/StdFile.h"

#include "mir/param/SimpleParametrisation.h"
#include "mir/config/LibMir.h"
#include "mir/method/WeightMatrix.h"

using mir::method::WeightMatrix;

namespace mir {
namespace caching {
namespace interpolator {

//----------------------------------------------------------------------------------------------------------------------

namespace {

const int MAGIC  =  987654321;
const size_t INFO_PATH = 1024;

struct SHM_Info_ {
    int ready;
    int magic;
    char path[INFO_PATH];
};


typedef eckit::Padded<SHM_Info_,1280> SHMInfo; /* aligned to 64 bytes */

}


class Unloader {

    std::vector<eckit::PathName> paths_;

public:

    /// This ensures unloader is destructed in correct order with other static objects (like eckit::Log)
    static Unloader& instance() {
        static Unloader unloader;
        return unloader;
    }

    void add(const eckit::PathName& path) {
        paths_.push_back(path);
    }

    ~Unloader() {
        for (std::vector<eckit::PathName>::const_iterator j = paths_.begin(); j != paths_.end(); ++j) {
            try {
                SharedMemoryLoader::unloadSharedMemory(*j);
            } catch (std::exception& e) {
                eckit::Log::info() << e.what() << std::endl;
            }
        }
    }
};

//----------------------------------------------------------------------------------------------------------------------


SharedMemoryLoader::SharedMemoryLoader(const std::string& name, const eckit::PathName& path) :
    InterpolatorLoader(name, path),
    address_(0),
    size_(0),
    unload_(false) {

    eckit::Log::debug<LibMir>() << "Loading shared memory interpolator from " << path << std::endl;

    /// FIXME size is based on file.size() -- which is assumed to be bigger than the memory footprint
    /** size_t sz = sizeof(SHMInfo) + w.footprint(); **/

    size_t sz = size_t(path.size()) + sizeof(SHMInfo);
    long page_size = ::sysconf(_SC_PAGESIZE);
    ASSERT(page_size > 0);
    size_t shmsize = eckit::round(sz, page_size);

    size_ = shmsize;

    unload_ = name.substr(0, 4) == "tmp-";

    eckit::TraceTimer<LibMir> timer("Loading interpolator coefficients from shared memory");
    eckit::PathName real = path.realName();

    // eckit::Log::debug<LibMir>() << "Loading interpolator coefficients from " << real << std::endl;

    if (real.asString().size() >= INFO_PATH - 1) {
        std::ostringstream os;
        os << "SharedMemoryLoader: path name to long " << real << ", maximum " << INFO_PATH;
        throw eckit::SeriousBug(os.str());
    }

    key_t key = ftok(real.asString().c_str(), 1);
    if (key == key_t(-1)) {
        throw eckit::FailedSystemCall("ftok(" + real.asString() + ")");
    }

     static const int max_wait_lock = eckit::Resource<int>("$MIR_SEMLOCK_RETRIES", 60);

    // Try to get an exclusing lock, we may be waiting for another process
    // to create the memory segment and load it with the file content
    int sem;
    SYSCALL(sem = ::semget(key, 1, IPC_CREAT | 0600));
    eckit::SemLocker locker(sem, real, max_wait_lock);

#ifdef IPC_INFO
    // Only on Linux?
    struct shminfo shm_info;
    SYSCALL(shmctl(0, IPC_INFO, reinterpret_cast<shmid_ds*>(&shm_info)));
    eckit::Log::debug<LibMir>() << "Maximum shared memory segment size: " << eckit::Bytes((shm_info.shmmax >> 10) * 1024) << std::endl;
#endif
    // This may return EINVAL is the segment is too large 256MB
    // To find the maximum:
    // Linux:ipcs -l, Mac/bsd: ipcs -M

    // eckit::Log::debug<LibMir>() << "SharedMemoryLoader: size is " << shmsize << " (" << eckit::Bytes(shmsize) << "), key=0x" <<
    //                          std::hex << key << std::dec << ", page size: "
    //                          << eckit::Bytes(page_size) << ", pages: "
    //                          << eckit::BigNum(shmsize / page_size)
    //                          << std::endl;

    int shmid;
    if ((shmid = shmget(key, shmsize , IPC_CREAT | 0600)) < 0) {
        std::ostringstream oss;
        oss << "Failed to aquire shared memory for " << eckit::Bytes(shmsize) << ", check the maximum authorised on this system (Linux ipcs -l, Mac/BSD ipcs -M)";
        throw eckit::FailedSystemCall(oss.str());
    }

    // Make sure memory is unloaded on exit
    if (unload_) {
        Unloader::instance().add(path);
    }

#ifdef SHM_PAGESIZE
    {

        // eckit::Log::debug<LibMir>() << "SharedMemoryLoader: attempting to use 64K pages"  << std::endl;

        /* Use 64K pages to back the shared memory region */
        size_t shm_size;
        struct shmid_ds shm_buf = { 0 };
        psize_t psize_64k;
        psize_64k = 64 * 1024;

        shm_buf.shm_pagesize = psize_64k;
        if (shmctl(shmid, SHM_PAGESIZE, &shm_buf)) {
            /*perror("shmctl(SHM_PAGESIZE) failed");*/
        }
    }

#endif

    /* attach shared memory */

    address_ = shmat( shmid, NULL, 0 );
    if (address_ == (void*) - 1) {
        throw eckit::FailedSystemCall("sfmat(" + real.asString() + ")");
    }

    try {

        char *addr = reinterpret_cast<char*>(address_);

        SHMInfo* nfo  = reinterpret_cast<SHMInfo*>(addr);

        // Check if the file has been loaded in memory
        bool load_matrix_to_memory = true;
        if (nfo->ready) {
            load_matrix_to_memory = false;
            if (nfo->magic != MAGIC) {
                std::ostringstream os;
                os << "SharedMemoryLoader: bad magic found " << nfo->magic;

                throw eckit::SeriousBug(os.str());
            }

            if (real.asString() != nfo->path) {
                std::ostringstream os;
                os << "SharedMemoryLoader: path mismatch " << real << " and " << nfo->path;

                throw eckit::SeriousBug(os.str());
            }
        }


        if (load_matrix_to_memory) {

            WeightMatrix w(path);

            w.dump(addr + sizeof(SHMInfo), size());

            // Set info record for checkes

            nfo->magic = MAGIC;
            strcpy(nfo->path, real.asString().c_str());
            nfo->ready = 1;

        }
        else {
            eckit::Log::debug<LibMir>() << "SharedMemoryLoader: " << path_ << " already loaded" << std::endl;
        }

    } catch (...) {
        shmdt(address_);
        throw;
    }
}


SharedMemoryLoader::~SharedMemoryLoader() {
    if (address_) {
        SYSCALL(shmdt(address_));
    }
    if (unload_) {
        unloadSharedMemory(path_);
    }
}

void SharedMemoryLoader::loadSharedMemory(const eckit::PathName& path, method::WeightMatrix& W) {

    SharedMemoryLoader loader("shmem", path);

    WeightMatrix w(const_cast<void*>(loader.address()), loader.size());

    std::swap(w, W);
}

void SharedMemoryLoader::unloadSharedMemory(const eckit::PathName& path) {

    eckit::Log::debug<LibMir>() << "Unloading SharedMemory from " << path << std::endl;

    eckit::PathName real = path.realName();
    int shmid = 0;
    key_t key;

    key = ftok(real.asString().c_str(), 1);
    if (key == key_t(-1)) {
        throw eckit::FailedSystemCall("ftok(" + real.asString() + ")");
    }

    shmid = shmget(key, 0, 0600);
    if (shmid < 0 && errno != ENOENT) {
        // throw eckit::FailedSystemCall("Cannot get shared memory for " + path);
        eckit::Log::info() << "Cannot get shared memory for " << path << eckit::Log::syserr << std::endl;
        return;
    }

    if (shmid < 0 && errno == ENOENT) {
        // eckit::Log::info() << "SharedMemory from " << path  << " already unloaded" <<std::endl;
    } else {
        if (shmctl(shmid, IPC_RMID, 0) < 0) {
            eckit::Log::info() << "Cannot delete memory for " << path << eckit::Log::syserr << std::endl;
        }

        eckit::Log::debug<LibMir>() << "Succefully unloaded SharedMemory from " << path << std::endl;
    }
#if 0
    sem = semget(key, 1, 0600);
    if (sem < 0 && errno != ENOENT) {
        eckit::Log::info() << "Cannot get shared shemaphore for " << path << eckit::Log::syserr << std::endl;
        return;
    }

    if (sem < 0 && errno == ENOENT) {
        // eckit::Log::info() << "SharedMemory semaphore for " << path  << " already unloaded" <<std::endl;
    } else {
        if (semctl(sem, 0, IPC_RMID, 0) < 0) {
            eckit::Log::info() << "Cannot delete semaphore for " << path << eckit::Log::syserr << std::endl;
        }
        // eckit::Log::info() << "SharedMemory removed semaphore for " << path  <<std::endl;
    }
#endif
}

void SharedMemoryLoader::print(std::ostream &out) const {
    out << "SharedMemoryLoader[path=" << path_ << ",size=" << eckit::Bytes(size_) << ",unload=" << unload_ << "]";
}

const void* SharedMemoryLoader::address() const {
    return reinterpret_cast<const char*>(address_) + sizeof(SHMInfo);
}

size_t SharedMemoryLoader::size() const {
    return size_ -  sizeof(SHMInfo);
}

namespace {

static InterpolatorLoaderBuilder<SharedMemoryLoader> loader1("shared-memory");
static InterpolatorLoaderBuilder<SharedMemoryLoader> loader2("shmem");
static InterpolatorLoaderBuilder<SharedMemoryLoader> loader3("tmp-shmem");
static InterpolatorLoaderBuilder<SharedMemoryLoader> loader5("tmp-shared-memory");

}


//----------------------------------------------------------------------------------------------------------------------

}  // namespace interpolator
}  // namespace caching
}  // namespace mir

