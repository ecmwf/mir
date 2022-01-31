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


#include "mir/caching/legendre/MappedMemoryLoader.h"

#include <ostream>

#include <fcntl.h>
#include <sys/mman.h>

#include "eckit/memory/MMap.h"
#include "eckit/os/Stat.h"

#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Types.h"


namespace mir {
namespace caching {
namespace legendre {


MappedMemoryLoader::MappedMemoryLoader(const param::MIRParametrisation& parametrisation, const eckit::PathName& path) :
    LegendreLoader(parametrisation, path), fd_(-1), address_(nullptr), size_(0) {

    ASSERT(sizeof(size_) > 4);

    fd_ = ::open(path.localPath(), O_RDONLY);
    if (fd_ < 0) {
        Log::error() << "open(" << path << ')' << Log::syserr << std::endl;
        throw exception::FailedSystemCall("open");
    }

    eckit::Stat::Struct s;
    SYSCALL(eckit::Stat::stat(path.localPath(), &s));

    ASSERT(s.st_size > 0);
    size_ = size_t(s.st_size);

    address_ = eckit::MMap::mmap(nullptr, size_, PROT_READ, MAP_SHARED, fd_, 0);
    if (address_ == MAP_FAILED) {
        Log::error() << "open(" << path << ',' << size_ << ')' << Log::syserr << std::endl;
        throw exception::FailedSystemCall("mmap");
    }
}

MappedMemoryLoader::~MappedMemoryLoader() {
    if (address_ != nullptr) {
        SYSCALL(eckit::MMap::munmap(address_, size_));
    }
    if (fd_ >= 0) {
        SYSCALL(::close(fd_));
    }
}

void MappedMemoryLoader::print(std::ostream& out) const {
    out << "MappedMemoryLoader[path=" << path_ << ",size=" << Log::Bytes(size_) << "]";
}

const void* MappedMemoryLoader::address() const {
    return address_;
}

size_t MappedMemoryLoader::size() const {
    return size_;
}

bool MappedMemoryLoader::inSharedMemory() const {
    return true;
}

bool MappedMemoryLoader::shared() {
    return true;
}


static const LegendreLoaderBuilder<MappedMemoryLoader> loader1("mapped-memory");
static const LegendreLoaderBuilder<MappedMemoryLoader> loader2("mmap");


}  // namespace legendre
}  // namespace caching
}  // namespace mir
