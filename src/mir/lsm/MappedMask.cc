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
/// @date Apr 2015


#include "MappedMask.h"

#include <cmath>
#include <fcntl.h>
#include <sys/mman.h>

#include "eckit/io/StdFile.h"
#include "eckit/log/Bytes.h"
#include "eckit/log/Timer.h"
#include "eckit/os/Stat.h"
#include "eckit/utils/MD5.h"

#include "mir/config/LibMir.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"

#include "eckit/memory/MMap.h"

using eckit::MMap;

// On CRAY/Brodwell, the rounding of areas is incorrect
// 90 is actually 90 +- 1e-14
// #pragma GCC target ("no-fma")

//----------------------------------------------------------------------------------------------------------------------

namespace {

class FDClose {
    int fd_;
public:
    FDClose(int fd): fd_(fd) {}
    ~FDClose() {
        SYSCALL(::close(fd_));
    }
};

class Unmapper {
    void *address_;
    size_t size_;
public:
    Unmapper(void *address, size_t size): address_(address), size_(size) {}
    ~Unmapper() {
        SYSCALL(MMap::munmap(address_, size_));
    }
};

static const unsigned int MASKS[] = {1 << 7, 1 << 6, 1 << 5, 1 << 4, 1 << 3, 1 << 2, 1 << 1, 1 << 0};

}  // namespace


//----------------------------------------------------------------------------------------------------------------------


namespace mir {
namespace lsm {


MappedMask::MappedMask(const eckit::PathName& path,
                       const param::MIRParametrisation& parametrisation,
                       const repres::Representation& representation,
                       const std::string& which) :
    Mask(path, parametrisation, representation, which) {
}


MappedMask::~MappedMask() {
}


bool MappedMask::active() const {
    return true;
}


bool MappedMask::cacheable() const {
    return true;
}


void MappedMask::hash(eckit::MD5&md5) const {
    Mask::hash(md5);
    md5.add("MappedMask");
}


void MappedMask::print(std::ostream &out) const {
    out << "MappedMask[";
    Mask::print(out);
    out << "]";
}


const std::vector<bool>& MappedMask::mask() const {

    // Lazy loading
    if (!mask_.empty()) {
        eckit::Log::debug<LibMir>() << "MappedMask: passive loading" << std::endl;
        return mask_;
    }

    eckit::Log::debug<LibMir>() << "MappedMask: loading '" << path_  << "'" << std::endl;

    int fd = ::open(path_.localPath(), O_RDONLY);
    if (fd < 0) {
        eckit::Log::error() << "open(" << path_ << ')' << eckit::Log::syserr << std::endl;
        throw eckit::FailedSystemCall("open");
    }

    FDClose close(fd);

    eckit::Stat::Struct s;
    SYSCALL(eckit::Stat::stat(path_.localPath(), &s));

    ASSERT(s.st_size > 0);
    size_t size = size_t(s.st_size);


    void *address = MMap::mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);
    if (address == MAP_FAILED) {
        eckit::Log::error() << "open(" << path_ << ',' << size << ')'
                            << eckit::Log::syserr << std::endl;
        throw eckit::FailedSystemCall("mmap");
    }

    Unmapper unmap(address, size);

    size_t bits = size * 8;
    size_t Nj = size_t(std::sqrt(bits / 2));
    size_t Ni = Nj * 2;

    ASSERT(Ni * Nj / 8 == size);


    const size_t ROWS = Nj;
    const size_t COLS = Ni;

    eckit::Log::debug<LibMir>() << "LSM: Ni=" << Ni << ", Nj=" << Nj << std::endl;

    {
        eckit::TraceTimer<LibMir> timer("MappedMask: extract points");


        // NOTE: this is not using 3D coordinate systems
        // mask_.reserve(grid.size());

        const unsigned char *mask = reinterpret_cast<unsigned char *>(address);

        eckit::ScopedPtr<repres::Iterator> iter(representation_.iterator());
        while (iter->next()) {
            const repres::Iterator::point_ll_t& p = iter->pointUnrotated();
            Latitude lat = p.lat;
            Longitude lon = p.lon.normalise(Longitude::GREENWICH);

            ASSERT(lat >= Latitude::SOUTH_POLE);
            ASSERT(lat <= Latitude::NORTH_POLE);

            int row = int((90.0 - lat.value()) * (ROWS - 1) / Latitude::GLOBE.value());
            ASSERT(row >= 0 && row < int(ROWS));

            int col = int(lon.value() * COLS / Longitude::GLOBE.value());
            ASSERT(col >= 0 && col < int(COLS));

            size_t pos = COLS * size_t(row) + size_t(col);
            size_t byte = pos / 8;
            size_t bit = pos % 8;

            // TODO: Check me
            mask_.push_back((mask[byte] & MASKS[bit]) != 0);
        }
    }

    ASSERT(mask_.size());
    return mask_;
}

//----------------------------------------------------------------------------------------------------------------------


}  // namespace lsm
}  // namespace mir

