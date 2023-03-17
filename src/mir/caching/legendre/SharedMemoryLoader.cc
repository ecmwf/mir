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


#include "mir/caching/legendre/SharedMemoryLoader.h"

#include <fcntl.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <sstream>

// #include "eckit/config/Resource.h"
#include "eckit/io/StdFile.h"
#include "eckit/memory/Shmget.h"
// #include "eckit/os/SemLocker.h"
#include "eckit/runtime/Main.h"

#include "mir/config/LibMir.h"
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Error.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Trace.h"


namespace mir::caching::legendre {


namespace {

const int MAGIC        = 1234567890;
const size_t INFO_PATH = 1024;

struct SHMInfo {
    int ready;
    int magic;
    char path[INFO_PATH];
};

}  // namespace


class Unloader {

    std::vector<eckit::PathName> paths_;

    Unloader() = default;

public:
    /// This ensures unloader is destructed in correct order with other static objects (like Log)
    static Unloader& instance() {
        static Unloader unloader;
        return unloader;
    }

    void add(const eckit::PathName& path) { paths_.push_back(path); }

    Unloader(const Unloader&)            = delete;
    Unloader(Unloader&&)                 = delete;
    Unloader& operator=(const Unloader&) = delete;
    Unloader& operator=(Unloader&&)      = delete;

    ~Unloader() {
        for (auto& path : paths_) {
            try {
                SharedMemoryLoader::unloadSharedMemory(path);
            }
            catch (std::exception& e) {
                Log::error() << e.what() << std::endl;
            }
        }
    }
};


#if 0
class GlobalSemaphore {
public:
    GlobalSemaphore(eckit::PathName path) : path_(path) {

        Log::debug() << "Semaphore for " << path << std::endl;

        key_t key = ::ftok(path.asString().c_str(), 1);
        if (key == key_t(-1)) {
            throw exception::FailedSystemCall("ftok(" + path.asString() + ")");
        }
        SYSCALL(semaphore_ = ::semget(key, 1, IPC_CREAT | 0600));
    }

    eckit::PathName path_;
    int semaphore_;
};
#endif


SharedMemoryLoader::SharedMemoryLoader(const param::MIRParametrisation& parametrisation, const eckit::PathName& path) :
    LegendreLoader(parametrisation, path), address_(nullptr), size_(size_t(path.size())), unload_(false) {

    trace::Timer timer("SharedMemoryLoader: loading '" + path.asString() + "'");

    std::string name = LibMir::cacheLoader(LibMir::cache_loader::LEGENDRE);
    if (parametrisation.get("legendre-loader", name)) {
        unload_ = name.substr(0, 4) == "tmp-";
    }

    eckit::PathName real = path.realName();

    std::ostringstream msg("SharedMemoryLoader: ");

    msg << "path='" << real << "', hostname='" << eckit::Main::hostname() << "'";
    Log::info() << msg.str() << std::endl;

    if (real.asString().size() >= INFO_PATH - 1) {
        Log::warning() << msg.str() << ", path name too long, maximum=" << INFO_PATH;
        throw exception::SeriousBug(msg.str());
    }

    // Try to get an exclusive lock, we may be waiting for another process
    // to create the memory segment and load it with the file content
    //    GlobalSemaphore gsem(real.dirName());
    //    static const int max_wait_lock = eckit::Resource<int>("$MIR_SEMLOCK_RETRIES", 60);
    //    eckit::SemLocker locker(gsem.semaphore_, gsem.path_, max_wait_lock);

    key_t key = ::ftok(real.asString().c_str(), 1);
    if (key == key_t(-1)) {
        Log::warning() << msg.str() << "::ftok(" << real.asString() << "), " << util::Error();
        throw exception::FailedSystemCall(msg.str());
    }

    long page_size = ::sysconf(_SC_PAGESIZE);
    ASSERT(page_size > 0);
    size_t shmsize = ((size_ + page_size - 1) / page_size) * page_size + sizeof(SHMInfo);

    msg << ", size: " << shmsize << " (" << Log::Bytes(shmsize) << "), key: 0x" << std::hex << key << std::dec
        << ", page size: " << Log::Bytes(page_size) << ", pages: " << Log::Pretty(shmsize / size_t(page_size));

#ifdef IPC_INFO
    // Only on Linux?
    struct shminfo shm_info;
    SYSCALL(::shmctl(0, IPC_INFO, reinterpret_cast<shmid_ds*>(&shm_info)));
    msg << ", maximum shared memory segment size: " << Log::Bytes((shm_info.shmmax >> 10) * 1024);
#endif

    // This may return EINVAL is the segment is too large 256MB
    int shmid;
    if ((shmid = eckit::Shmget::shmget(key, shmsize, IPC_CREAT | 0600)) < 0) {
        Log::warning()
            << msg.str()
            << ", shmget: failed to acquire shared memory, check the maximum authorised on this system (Linux ipcs "
               "-l, macOS/BSD ipcs -M), "
            << util::Error() << std::endl;
        throw exception::FailedSystemCall(msg.str());
    }
    msg << ", shmid=" << shmid;

#ifdef SHM_PAGESIZE
    {

        // Log::debug() << "SharedMemoryLoader: attempting to use 64K pages"  << std::endl;

        /* Use 64K pages to back the shared memory region */
        size_t shm_size;
        struct shmid_ds shm_buf = {0};
        psize_t psize_64k;
        psize_64k = 64 * 1024;

        shm_buf.shm_pagesize = psize_64k;
        if (::shmctl(shmid, SHM_PAGESIZE, &shm_buf)) {
            // Log::warning() << "SharedMemoryLoader: ::shmctl(SHM_PAGESIZE) failed, " << util::Error() << std::endl;
        }
    }

#endif

    // Attach shared memory
    address_ = eckit::Shmget::shmat(shmid, nullptr, 0);
    if (address_ == (void*)-1) {
        Log::warning() << msg.str() << ", shmat: failed to attach shared memory, " << util::Error() << std::endl;
        throw exception::FailedSystemCall(msg.str());
    }


    try {

        auto* addr = reinterpret_cast<char*>(address_);
        auto* nfo  = reinterpret_cast<SHMInfo*>(addr + (((size_ + page_size - 1) / page_size) * page_size));

        // Check if the file has been loaded in memory
        if (nfo->ready != 0) {
            Log::debug() << msg.str() << ", already loaded" << std::endl;

            if (nfo->magic != MAGIC) {
                Log::warning() << msg.str() << ", bad magic=" << nfo->magic << std::endl;
                throw exception::SeriousBug(msg.str());
            }

            if (real.asString() != nfo->path) {
                Log::warning() << msg.str() << ", path mismatch, file='" << nfo->path << "'" << std::endl;
                throw exception::SeriousBug(msg.str());
            }
        }
        else {
            trace::Timer("SharedMemoryLoader: loading into shared memory");

            eckit::AutoStdFile file(real);
            ASSERT(std::fread(address_, 1, size_, file) == size_);

            // Set info record for checks
            nfo->magic = MAGIC;
            std::strcpy(nfo->path, real.asString().c_str());
            nfo->ready = 1;
        }
    }
    catch (...) {
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
    //     Log::debug() << "LOAD IPCS " << line << std::endl;
    // }
}


SharedMemoryLoader::~SharedMemoryLoader() {
    if (address_ != nullptr) {
        SYSCALL(eckit::Shmget::shmdt(address_));
    }
    if (unload_) {
        unloadSharedMemory(path_);
    }
}


void SharedMemoryLoader::loadSharedMemory(const eckit::PathName& path) {
    param::SimpleParametrisation param;
    SharedMemoryLoader loader(param, path);
}


void SharedMemoryLoader::unloadSharedMemory(const eckit::PathName& path) {
    Log::debug() << "SharedMemoryLoader: unloading '" << path << "'" << std::endl;

    eckit::PathName real = path.realName();

    key_t key = ::ftok(real.asString().c_str(), 1);
    if (key == key_t(-1)) {
        Log::warning() << "SharedMemoryLoader: ::ftok(" << real.asString() << ")" << std::endl;
        throw exception::FailedSystemCall("SharedMemoryLoader: ::ftok");
    }

    int shmid = eckit::Shmget::shmget(key, 0, 0600);
    if (shmid < 0) {
        Log::warning() << "SharedMemoryLoader: shmget: path='" << path << "', "
                       << (errno == ENOENT ? "already unloaded" : "failed to acquire shared memory") << std::endl;
        return;
    }

    // FIXME: add to eckit::Shmget interface
    if (::shmctl(shmid, IPC_RMID, nullptr) < 0) {
        Log::warning() << "SharedMemoryLoader: ::shmctl: cannot delete '" << path << "'" << std::endl;
    }

    Log::debug() << "SharedMemoryLoader: successfully unloaded '" << path << "'" << std::endl;

    // eckit::StdPipe f("ipcs", "r");
    // char line[1024];s
    // while(fgets(line, sizeof(line), f)) {
    //     Log::debug() << "UNLOAD IPCS " << line << std::endl;
    // }
}


void SharedMemoryLoader::print(std::ostream& out) const {
    out << "SharedMemoryLoader[path=" << path_ << ",size=" << Log::Bytes(size_) << ",unload=" << unload_ << "]";
}


const void* SharedMemoryLoader::address() const {
    return address_;
}


size_t SharedMemoryLoader::size() const {
    return size_;
}


bool SharedMemoryLoader::inSharedMemory() const {
    return true;
}


bool SharedMemoryLoader::shared() {
    return true;
}


static const LegendreLoaderBuilder<SharedMemoryLoader> loader1("shared-memory");
static const LegendreLoaderBuilder<SharedMemoryLoader> loader2("shmem");
static const LegendreLoaderBuilder<SharedMemoryLoader> loader3("tmp-shmem");
static const LegendreLoaderBuilder<SharedMemoryLoader> loader5("tmp-shared-memory");


}  // namespace mir::caching::legendre
