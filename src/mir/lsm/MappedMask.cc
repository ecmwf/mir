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


#include "MappedMask.h"

#include <fcntl.h>
#include <sys/mman.h>

#include <memory>

#include "eckit/memory/MMap.h"
#include "eckit/os/Stat.h"
#include "eckit/utils/MD5.h"

#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Trace.h"


using eckit::MMap;


// On CRAY/Brodwell, the rounding of areas is incorrect
// 90 is actually 90 +- 1e-14
// #pragma GCC target ("no-fma")


namespace {


class FDClose {
    int fd_;

public:
    FDClose(int fd) : fd_(fd) {}
    FDClose(const FDClose&)            = delete;
    FDClose(FDClose&&)                 = delete;
    FDClose& operator=(const FDClose&) = delete;
    FDClose& operator=(FDClose&&)      = delete;
    ~FDClose() { SYSCALL(::close(fd_)); }
};

class Unmapper {
    void* address_;
    size_t size_;

public:
    Unmapper(void* address, size_t size) : address_(address), size_(size) {}
    Unmapper(const Unmapper&)            = delete;
    Unmapper(Unmapper&&)                 = delete;
    Unmapper& operator=(const Unmapper&) = delete;
    Unmapper& operator=(Unmapper&&)      = delete;
    ~Unmapper() { SYSCALL(MMap::munmap(address_, size_)); }
};


}  // namespace


static const unsigned int MASKS[] = {1 << 7, 1 << 6, 1 << 5, 1 << 4, 1 << 3, 1 << 2, 1 << 1, 1 << 0};


namespace mir {
namespace lsm {


MappedMask::MappedMask(const std::string& name, const eckit::PathName& path,
                       const param::MIRParametrisation& /*unused*/, const repres::Representation& representation,
                       const std::string& /*unused*/) :
    name_(name), path_(path) {

    int fd = ::open(path_.localPath(), O_RDONLY);
    if (fd < 0) {
        Log::error() << "open(" << path_ << ')' << Log::syserr << std::endl;
        throw exception::FailedSystemCall("open");
    }

    FDClose close(fd);

    eckit::Stat::Struct s;
    SYSCALL(eckit::Stat::stat(path_.localPath(), &s));

    auto size = size_t(s.st_size);

    void* address = MMap::mmap(nullptr, size, PROT_READ, MAP_SHARED, fd, 0);
    if (address == MAP_FAILED) {
        Log::error() << "open(" << path_ << ',' << size << ')' << Log::syserr << std::endl;
        throw exception::FailedSystemCall("mmap");
    }

    Unmapper unmap(address, size);

    size_t bits = size * 8;
    auto Nj     = size_t(std::sqrt(bits / 2));
    auto Ni     = Nj * 2;

    ASSERT(Ni * Nj / 8 == size);

    const size_t ROWS = Nj;
    const size_t COLS = Ni;

    Log::debug() << "LSM: Ni=" << Ni << ", Nj=" << Nj << std::endl;

    trace::Timer timer("Extract points from  LSM");

    // NOTE: this is not using 3D coordinate systems
    // mask_.reserve(grid.size());

    const unsigned char* mask = reinterpret_cast<unsigned char*>(address);

    for (const std::unique_ptr<repres::Iterator> it(representation.iterator()); it->next();) {
        const auto& p = it->pointUnrotated();
        Latitude lat  = p.lat();
        Longitude lon = p.lon().normalise(Longitude::GREENWICH);

        if (lat < Latitude::SOUTH_POLE) {
            auto msg = "GRID  returns a latitude of " + std::to_string(lat.value()) +
                       " (lat+90)=" + std::to_string((lat + Latitude::NORTH_POLE).value());
            throw exception::SeriousBug(msg);
        }
        ASSERT(lat >= Latitude::SOUTH_POLE);

        if (lat > Latitude::NORTH_POLE) {
            auto msg = "GRID  returns a latitude of " + std::to_string(lat.value()) +
                       " (lat-90)=" + std::to_string((lat + Latitude::SOUTH_POLE).value());
            throw exception::SeriousBug(msg);
        }
        ASSERT(lat <= Latitude::NORTH_POLE);

        int row = int((90.0 - lat.value()) * double(ROWS - 1) / Latitude::GLOBE.value());
        ASSERT(row >= 0 && row < int(ROWS));

        int col = int(lon.value() * double(COLS) / Longitude::GLOBE.value());
        ASSERT(col >= 0 && col < int(COLS));

        size_t pos  = COLS * size_t(row) + size_t(col);
        size_t byte = pos / 8;
        size_t bit  = pos % 8;

        // TODO: Check me
        mask_.push_back((mask[byte] & MASKS[bit]) != 0);
    }
}


MappedMask::~MappedMask() = default;


bool MappedMask::active() const {
    return true;
}


bool MappedMask::cacheable() const {
    return true;
}


void MappedMask::hash(eckit::MD5& md5) const {
    Mask::hash(md5);
    md5.add(path_.asString());
}


void MappedMask::print(std::ostream& out) const {
    out << "MappedMask[path=" << path_ << "]";
}


std::string MappedMask::cacheName() const {
    return name_;
}


const std::vector<bool>& MappedMask::mask() const {
    return mask_;
}


}  // namespace lsm
}  // namespace mir
