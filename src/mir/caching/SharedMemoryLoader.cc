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
/// @date Apr 2015


#include <sys/sem.h>

#include "mir/caching/SharedMemoryLoader.h"
#include "mir/param/SimpleParametrisation.h"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

#include <sys/sem.h>

#include "eckit/eckit.h"
#include "eckit/os/Stat.h"

#include "eckit/log/Bytes.h"
#include "eckit/log/BigNum.h"

#include "eckit/log/Timer.h"
#include "eckit/io/StdFile.h"
#include "mir/log/MIR.h"


namespace mir {
namespace caching {

namespace {
struct sembuf _lock[] = {
    { 0, 0,  SEM_UNDO }, /* test */
    { 0, 1,  SEM_UNDO }, /* lock */
};

struct sembuf _unlock[] = {
    { 0, -1, SEM_UNDO }, /* ulck */
};

const int MAGIC  =  1234567890;
const size_t INFO_PATH = 1024;
struct info {
    int ready;
    int magic;
    char path[INFO_PATH];
};


class AutoFDClose {
    int fd_;
  public:
    AutoFDClose(int fd): fd_(fd) {}
    ~AutoFDClose() {
        ::close(fd_);
    }
};


class SemLocker {

    static const int MAX_WAIT_LOCK = 30;
    static const int SLEEP = 2;

    int sem_;
    eckit::PathName path_;

  public:

    SemLocker(int s, const eckit::PathName& path) :
        sem_(s),
        path_(path) {

        int retry = 0;
        while (retry < MAX_WAIT_LOCK) {
            if (semop(sem_, _lock, 2 ) < 0) {
                eckit::Log::warning() << "SharedMemoryLoader: Failed to acquire exclusive lock on " << path_ << std::endl;

                retry++;
                // sprintf(message,"ERR: sharedmem:semop:lock(%s)",path);
                if (retry >= MAX_WAIT_LOCK) {
                    std::ostringstream os;
                    os << "Failed to acquire semaphore lock for " << path_;
                    throw eckit::SeriousBug(os.str());
                } else {
                    eckit::Log::warning() << "Sleeping for " << SLEEP << " seconds" << std::endl;
                    sleep(SLEEP);
                    retry++;
                }
            } else {
                break;
            }
        }
    }

    ~SemLocker() {
        int retry = 0;
        while (retry < MAX_WAIT_LOCK) {
            if (semop(sem_, _unlock, 1) < 0) {
                if (retry >= MAX_WAIT_LOCK) {
                    std::ostringstream os;
                    os << "Failed to realease semaphore lock for " << path_;
                    throw eckit::SeriousBug(os.str());
                } else {
                    eckit::Log::warning() << "Sleeping for " << SLEEP << " seconds" << std::endl;
                    sleep(SLEEP);
                    retry++;
                }
            } else {
                break;
            }
        }
    }

};

}


class Unloader {
    std::vector<eckit::PathName> paths_;
  public:
    void add(const eckit::PathName& path) {
        paths_.push_back(path);
    }

    ~Unloader() {
        for (std::vector<eckit::PathName>::const_iterator j = paths_.begin(); j != paths_.end(); ++j) {
            try {
                SharedMemoryLoader::unloadSharedMemory(*j);
            } catch (std::exception& e) {
                std::cout << e.what() << std::endl;
            }
        }
    }
};

static Unloader unloader;

SharedMemoryLoader::SharedMemoryLoader(const param::MIRParametrisation &parametrisation, const eckit::PathName &path):
    LegendreLoader(parametrisation, path),
    address_(0),
    size_(path.size()),
    unload_(false) {

    // eckit::Log::info() << "Loading shared memory from " << path << std::endl;


    std::string name;

    if (parametrisation.get("legendre-loader", name)) {
        unload_ = name.substr(0, 4) == "tmp-";
    }

    if (unload_) {
        unloader.add(path);
    }

    // eckit::TraceTimer<MIR> timer("Loading legendre coefficients from shared memory");
    eckit::PathName real = path.realName();
    // eckit::Log::trace<MIR>() << "Loading legendre coefficients from " << real << std::endl;

    if (real.asString().size() >= INFO_PATH - 1) {
        std::ostringstream os;
        os << "SharedMemoryLoader: path name to long " << real << ", maximum " << INFO_PATH;
        throw eckit::SeriousBug(os.str());
    }

    key_t key = ftok(real.asString().c_str(), 1);
    if (key == key_t(-1)) {
        throw eckit::FailedSystemCall("ftok(" + real.asString() + ")");
    }


    // Try to get an exclusing lock, we may be waiting for another process
    // to create the memory segment and load it with the file content
    int sem;
    SYSCALL(sem = semget(key, 1, IPC_CREAT | 0600));
    SemLocker locker(sem, real);

    // O_LARGEFILE ?
    int fd;
    SYSCALL(fd = ::open(real.asString().c_str(), O_RDONLY));
    AutoFDClose c(fd);

    int page_size = getpagesize();
    ASSERT(page_size > 0);
    size_t shmsize = ((size_ + page_size - 1) / page_size) * page_size + sizeof(struct info) ;

#ifdef IPC_INFO
    // Only on Linux?
    struct shminfo shm_info;
    SYSCALL(shmctl(0, IPC_INFO, reinterpret_cast<shmid_ds*>(&shm_info)));
    eckit::Log::trace<MIR>() << "Maximum shared memory segment size: " << eckit::Bytes((shm_info.shmmax >> 10) * 1024) << std::endl;
#endif
    // This may return EINVAL is the segment is too large 256MB
    // To find the maximum:
    // Linux:ipcs -l, Mac/bsd: ipcs -M

    eckit::Log::trace<MIR>() << "SharedMemoryLoader: size is " << shmsize << " (" << eckit::Bytes(shmsize) << "), key=0x" <<
                             std::hex << key << std::dec << ", page size: "
                             << eckit::Bytes(page_size) << ", pages: "
                             << eckit::BigNum(shmsize / page_size)
                             << std::endl;

    int shmid;
    SYSCALL(shmid = shmget(key, shmsize , IPC_CREAT | 0600)) ;

#ifdef SHM_PAGESIZE
    {

        eckit::Log::trace<MIR>() << "SharedMemoryLoader: attempting to use 64K pages"  << std::endl;

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

        info* nfo  = reinterpret_cast<info*>(addr + (((size_ + page_size - 1) / page_size) * page_size));

        // Check if the file has been loaded in memory
        bool loadfile = true;
        if (nfo->ready) {
            loadfile = false;
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


        if (loadfile) {
            // eckit::Log::info() << "SharedMemoryLoader: loading " << path_ << std::endl;
            eckit::Timer("Loading file into shared memory");
            eckit::StdFile file(real);
            ASSERT(::fread(address_, 1, size_, file) == size_);

            // Set info record for checkes

            nfo->magic = MAGIC;
            strcpy(nfo->path, real.asString().c_str());
            nfo->ready = 1;
        } else {
            // eckit::Log::info() << "SharedMemoryLoader: " << path_ << " already loaded" << std::endl;
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

void SharedMemoryLoader::loadSharedMemory(const eckit::PathName& path) {
    param::SimpleParametrisation param;
    SharedMemoryLoader loader(param, path);
}

void SharedMemoryLoader::unloadSharedMemory(const eckit::PathName& path) {
    // std::cout << "Unloading SharedMemory from " << path << std::endl;

    eckit::PathName real = path.realName();
    int shmid = 0;
    key_t key;
    int sem;

    key = ftok(real.asString().c_str(), 1);
    if (key == key_t(-1)) {
        throw eckit::FailedSystemCall("ftok(" + real.asString() + ")");
    }

    shmid = shmget(key, 0, 0600);
    if (shmid < 0 && errno != ENOENT) {
        throw eckit::FailedSystemCall("Cannot get shared memory for " + path);
    }

    if (shmid < 0 && errno == ENOENT) {
        // std::cout << "SharedMemory from " << path  << " already unloaded" <<std::endl;
    } else {
        SYSCALL(shmctl(shmid, IPC_RMID, 0));
        // std::cout << "Succefully unloaded SharedMemory from " << path  << std::endl;
    }

    sem = semget(key, 1, 0600);
    if (sem < 0 && errno != ENOENT) {
        throw eckit::FailedSystemCall("Cannot get shared semaphor for " + path);
    }

    if (sem < 0 && errno == ENOENT) {
        // std::cout << "SharedMemory semaphore for " << path  << " already unloaded" <<std::endl;
    } else {
        SYSCALL(semctl(sem, 0, IPC_RMID, 0));
        // std::cout << "SharedMemory removed semaphore for " << path  <<std::endl;
    }

}

void SharedMemoryLoader::print(std::ostream &out) const {
    out << "SharedMemoryLoader[path=" << path_ << ",size=" << eckit::Bytes(size_) << ",unload=" << unload_ << "]";
}

const void *SharedMemoryLoader::address() const {
    return address_;
}

size_t SharedMemoryLoader::size() const {
    return size_;
}

namespace {
static LegendreLoaderBuilder<SharedMemoryLoader> loader1("shared-memory");
static LegendreLoaderBuilder<SharedMemoryLoader> loader2("shmem");
static LegendreLoaderBuilder<SharedMemoryLoader> loader3("tmp-shmem");
static LegendreLoaderBuilder<SharedMemoryLoader> loader5("tmp-shared-memory");


}


}  // namespace caching
}  // namespace mir

