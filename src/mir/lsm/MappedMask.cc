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

#include <sys/mman.h>
#include <fcntl.h>

#include <cmath>

#include "atlas/Grid.h"
#include "eckit/io/StdFile.h"
#include "eckit/log/Bytes.h"
#include "eckit/log/Timer.h"
#include "eckit/os/Stat.h"


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
        SYSCALL(::munmap(address_, size_));
    }
};

static const unsigned int MASKS[] = {1 << 7, 1 << 6, 1 << 5, 1 << 4, 1 << 3, 1 << 2, 1 << 1, 1 << 0};

}  // namespace

namespace mir {
namespace lsm {



MappedMask::MappedMask(const std::string &name,
                       const param::MIRParametrisation &parametrisation,
                       const atlas::Grid &grid,
                       const std::string &which):
    Mask(name),
    path_("~mir/etc/1km-lsm.mask") {



    int fd = ::open(path_.localPath(), O_RDONLY);
    if (fd < 0) {
        eckit::Log::error() << "open(" << path_ << ')' << eckit::Log::syserr << std::endl;
        throw eckit::FailedSystemCall("open");
    }

    FDClose close(fd);

    eckit::Stat::Struct s;
    SYSCALL(eckit::Stat::stat(path_.localPath(), &s));

    size_t size = s.st_size;


    void *address = ::mmap(0, size, PROT_READ, MAP_SHARED, fd, 0);
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


    const size_t ROWS = Ni;
    const size_t COLS = Nj;

    eckit::Log::info() << "LSM: Ni=" << Ni << ", Nj=" << Nj << std::endl;

    eckit::Timer timer("Extract points from  LSM");


    // NOTE: this is not using 3D coordinate systems

    std::vector<atlas::Grid::Point> points(grid.npts());
    grid.lonlat(points);

    mask_.reserve(points.size());

    const unsigned char *mask = reinterpret_cast<unsigned char *>(address);

    for (std::vector<atlas::Grid::Point>::const_iterator j = points.begin(); j != points.end(); ++j) {
        double lat = (*j).lat();
        ASSERT(lat >= -90);
        ASSERT(lat <= 90);

        double lon = (*j).lon();

        while (lon >= 360) {
            lon -= 360;
        }
        while (lon < 0) {
            lon += 360;
        }

        int row = (90.0 - lat) * (ROWS - 1) / 180;
        ASSERT(row >= 0 && row < ROWS);

        int col = lon * COLS / 360.0;
        ASSERT(col >= 0 && col < COLS);

        size_t pos = COLS * row + col;
        size_t byte = pos / 8;
        size_t bit = pos % 8;

        // TODO: Check me
        mask_.push_back((mask[byte] & MASKS[bit]) != 0);
    }

}

MappedMask::~MappedMask() {
}

void MappedMask::hash(eckit::MD5 &md5) const {
    Mask::hash(md5);
    md5.add(path_.asString());
}

void MappedMask::print(std::ostream &out) const {
    out << "MappedMask[path=" << path_ << "]";
}


const std::vector<bool> &MappedMask::mask() const {
    return mask_;
}

//-----------------------------------------------------------------------------


}  // namespace logic
}  // namespace mir

