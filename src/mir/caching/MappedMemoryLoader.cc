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


#include <iostream>

#include "mir/caching/MappedMemoryLoader.h"

#include <sys/mman.h>
#include <fcntl.h>

#include "eckit/eckit.h"
#include "eckit/os/Stat.h"

#include "eckit/log/Bytes.h"


namespace mir {
namespace caching {


MappedMemoryLoader::MappedMemoryLoader(const param::MIRParametrisation &parametrisation, const eckit::PathName &path):
    LegendreLoader(parametrisation, path),
    fd_(-1),
    size_(0),
    address_(0) {

    ASSERT(sizeof(size_) > 4);

    fd_ = ::open(path.localPath(), O_RDONLY);
    if (fd_ < 0) {
        eckit::Log::error() << "open(" << path << ')' << eckit::Log::syserr << std::endl;
        throw eckit::FailedSystemCall("open");
    }

    eckit::Stat::Struct s;
    SYSCALL(eckit::Stat::stat(path.localPath(), &s));

    size_ = s.st_size;


    address_ = ::mmap(0, size_, PROT_READ, MAP_SHARED, fd_, 0);
    if (address_ == MAP_FAILED) {
        eckit::Log::error() << "open(" << path << ',' << size_ << ')'
                            << eckit::Log::syserr << std::endl;
        throw eckit::FailedSystemCall("mmap");
    }


}


MappedMemoryLoader::~MappedMemoryLoader() {
    if (address_)
        SYSCALL(::munmap(address_, size_));
    if (fd_ >= 0)
        SYSCALL(::close(fd_));
}


void MappedMemoryLoader::print(std::ostream &out) const {
    out << "MappedMemoryLoader[path=" << path_ << ",size=" << eckit::Bytes(size_) << "]";
}

const void *MappedMemoryLoader::address() const {
    return address_;
}

size_t MappedMemoryLoader::size() const {
    return size_;
}

namespace {
static LegendreLoaderBuilder<MappedMemoryLoader> loader1("mapped-memory");
static LegendreLoaderBuilder<MappedMemoryLoader> loader2("mmap");

}


}  // namespace caching
}  // namespace mir

