/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/caching/matrix/SharedMemoryLoader.h"

#include <cerrno>
#include <cstring>

#include <fcntl.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/sem.h>

//#include "eckit/config/Resource.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/log/Bytes.h"
#include "eckit/log/Timer.h"
#include "eckit/maths/Functions.h"
#include "eckit/memory/Padded.h"
#include "eckit/memory/Shmget.h"
//#include "eckit/os/SemLocker.h"
#include "eckit/runtime/Main.h"

#include "mir/method/WeightMatrix.h"
#include "mir/util/Error.h"
#include "mir/util/Pretty.h"


namespace mir {
namespace caching {
namespace matrix {


namespace {

const int MAGIC  =  987654321;
const size_t INFO_PATH = 1024;

struct SHMInfoNotPadded {
    int ready;
    int magic;
    char path[INFO_PATH];
};

using SHMInfo = eckit::Padded<SHMInfoNotPadded, 1280>;  // aligned to 64 bytes

}


class Unloader {

    std::vector<eckit::PathName> paths_;

public:
    /// This ensures unloader is destructed in correct order with other static objects (like eckit::Log)
    static Unloader& instance() {
        static Unloader unloader;
        return unloader;
    }

    void add(const eckit::PathName& path) { paths_.push_back(path); }

    ~Unloader() {
        for (auto& path : paths_) {
            try {
                SharedMemoryLoader::unloadSharedMemory(path);
            }
            catch (std::exception& e) {
                eckit::Log::error() << e.what() << std::endl;
            }
        }
    }
};


#if 0
class GlobalSemaphore {
public:
    GlobalSemaphore(eckit::PathName path) : path_(path) {

        SharedMemoryLoader::log() << "Semaphore for " << path << std::endl;

        key_t key = ::ftok(path.asString().c_str(), 1);
        if (key == key_t(-1)) {
            throw eckit::FailedSystemCall("ftok(" + path.asString() + ")");
        }
        SYSCALL(semaphore_ = ::semget(key, 1, IPC_CREAT | 0600));
    }

    eckit::PathName path_;
    int semaphore_;
};
#endif


SharedMemoryLoader::SharedMemoryLoader(const std::string& name, const eckit::PathName& path) :
    MatrixLoader(name, path),
    address_(nullptr),
    size_(0),
    unload_(false) {

    eckit::Timer timer("SharedMemoryLoader: loading '" + path.asString() + "'", log());

    unload_ = name.substr(0, 4) == "tmp-";

    eckit::PathName real = path.realName();

    std::ostringstream msg("SharedMemoryLoader: ");

    msg << "path='" << real << "', hostname='" << eckit::Main::hostname() << "'";
    log() << msg.str() << std::endl;

    if (real.asString().size() >= INFO_PATH - 1) {
        warn() << msg.str() << ", path name too long, maximum=" << INFO_PATH;
        throw eckit::SeriousBug(msg.str());
    }

    // Try to get an exclusive lock, we may be waiting for another process
    // to create the memory segment and load it with the file content
//    GlobalSemaphore gsem(real.dirName());
//    static const int max_wait_lock = eckit::Resource<int>("$MIR_SEMLOCK_RETRIES", 60);
//    eckit::SemLocker locker(gsem.semaphore_, gsem.path_, max_wait_lock);

    key_t key = ::ftok(real.asString().c_str(), 1);
    if (key == key_t(-1)) {
        warn() << msg.str() << "::ftok(" << real.asString() << "), " << util::Error();
        throw eckit::FailedSystemCall(msg.str());
    }

    // NOTE: size is based on file.size() which is assumed to be bigger than the memory footprint. Real size would be:
    // size_t sz = sizeof(SHMInfo) + w.footprint();

    size_t sz = size_t(path.size()) + sizeof(SHMInfo);
    long page_size = ::sysconf(_SC_PAGESIZE);
    ASSERT(page_size > 0);
    size_t shmsize = eckit::round(sz, page_size);

    size_ = shmsize;

    msg << ", size: " << shmsize << " (" << eckit::Bytes(shmsize) << "), key: 0x" << std::hex << key << std::dec
        << ", page size: " << eckit::Bytes(page_size) << ", pages: " << Pretty(shmsize / page_size);

#ifdef IPC_INFO
    // Only on Linux?
    struct shminfo shm_info;
    SYSCALL(::shmctl(0, IPC_INFO, reinterpret_cast<shmid_ds*>(&shm_info)));
    msg << ", maximum shared memory segment size: " << eckit::Bytes((shm_info.shmmax >> 10) * 1024);
#endif

    // This may return EINVAL is the segment is too large 256MB
    int shmid;
    if ((shmid = eckit::Shmget::shmget(key, shmsize, IPC_CREAT | 0600)) < 0) {
        warn() << msg.str()
               << ", shmget: failed to acquire shared memory, check the maximum authorised on this system (Linux ipcs "
                  "-l, macOS/BSD ipcs -M), "
               << util::Error() << std::endl;
        throw eckit::FailedSystemCall(msg.str());
    }
    msg << ", shmid=" << shmid << std::endl;

#ifdef SHM_PAGESIZE
    {

        // log() << "SharedMemoryLoader: attempting to use 64K pages"  << std::endl;

        /* Use 64K pages to back the shared memory region */
        size_t shm_size;
        struct shmid_ds shm_buf = { 0 };
        psize_t psize_64k;
        psize_64k = 64 * 1024;

        shm_buf.shm_pagesize = psize_64k;
        if (::shmctl(shmid, SHM_PAGESIZE, &shm_buf)) {
            // warn() << "SharedMemoryLoader: ::shmctl(SHM_PAGESIZE) failed, " << util::Error() << std::endl;
        }
    }

#endif

    // Attach shared memory
    address_ = eckit::Shmget::shmat(shmid, NULL, 0);
    if (address_ == (void*) - 1) {
        warn() << msg.str() << ", shmat: failed to attach shared memory, " << util::Error() << std::endl;
        throw eckit::FailedSystemCall(msg.str());
    }


    try {

        char* addr   = reinterpret_cast<char*>(address_);
        SHMInfo* nfo = reinterpret_cast<SHMInfo*>(addr);

        // Check if the file has been loaded in memory
        if (nfo->ready) {
            log() << msg.str() << ", already loaded" << std::endl;

            if (nfo->magic != MAGIC) {
                warn() << msg.str() << ", bad magic=" << nfo->magic << std::endl;
                throw eckit::SeriousBug(msg.str());
            }

            if (real.asString() != nfo->path) {
                warn() << msg.str() << ", path mismatch, file='" << nfo->path << "'" << std::endl;
                throw eckit::SeriousBug(msg.str());
            }
        }
        else {

            method::WeightMatrix w(path);
            w.dump(addr + sizeof(SHMInfo), size());

            // Set info record for checks
            nfo->magic = MAGIC;
            std::strcpy(nfo->path, real.asString().c_str());
            nfo->ready = 1;
        }

    } catch (...) {
        eckit::Shmget::shmdt(address_);
        throw;
    }

    // Make sure memory is unloaded on exit
    if (unload_) {
        Unloader::instance().add(path);
    }

    // eckit::StdPipe f("ipcs", "r");
    // char line[1024];

    // while(fgets(line, sizeof(line), f)) {
    //     log() << "LOAD IPCS " << line << std::endl;
    // }
}


SharedMemoryLoader::~SharedMemoryLoader() {
    if (address_) {
        SYSCALL(eckit::Shmget::shmdt(address_));
    }
    if (unload_) {
        unloadSharedMemory(path_);
    }
}


void SharedMemoryLoader::loadSharedMemory(const eckit::PathName& path) {
    SharedMemoryLoader loader("shmem", path);
}


void SharedMemoryLoader::unloadSharedMemory(const eckit::PathName& path) {
    log() << "SharedMemoryLoader: unloading '" << path << "'" << std::endl;

    eckit::PathName real = path.realName();

    key_t key = ::ftok(real.asString().c_str(), 1);
    if (key == key_t(-1)) {
        warn() << "SharedMemoryLoader: ::ftok(" << real.asString() << ")" << std::endl;
        throw eckit::FailedSystemCall("SharedMemoryLoader: ::ftok");
    }

    int shmid = eckit::Shmget::shmget(key, 0, 0600);
    if (shmid < 0) {
        warn() << "SharedMemoryLoader: shmget: path='" << path << "', "
               << (errno == ENOENT ? "already unloaded" : "failed to acquire shared memory") << std::endl;
        return;
    }

    // FIXME: add to eckit::Shmget interface
    if (::shmctl(shmid, IPC_RMID, 0) < 0) {
        warn() << "SharedMemoryLoader: ::shmctl: cannot delete '" << path << "'" << std::endl;
    }

    log() << "SharedMemoryLoader: successfully unloaded '" << path << "'" << std::endl;

    // eckit::StdPipe f("ipcs", "r");
    // char line[1024];s
    // while(fgets(line, sizeof(line), f)) {
    //     log() << "UNLOAD IPCS " << line << std::endl;
    // }
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


bool SharedMemoryLoader::inSharedMemory() const {
    return true;
}


namespace {
static MatrixLoaderBuilder<SharedMemoryLoader> loader1("shared-memory");
static MatrixLoaderBuilder<SharedMemoryLoader> loader2("shmem");
static MatrixLoaderBuilder<SharedMemoryLoader> loader3("tmp-shmem");
static MatrixLoaderBuilder<SharedMemoryLoader> loader5("tmp-shared-memory");
}


}  // namespace matrix
}  // namespace caching
}  // namespace mir

