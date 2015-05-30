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


#include "TenMinutesLSM.h"

#include "eckit/filesystem/PathName.h"
#include "eckit/log/Timer.h"

#include "mir/input/GribFileInput.h"
#include "mir/data/MIRField.h"
#include "mir/method/Method.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/repres/Representation.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/io/StdFile.h"
#include "atlas/Grid.h"

namespace {


static eckit::Mutex local_mutex ;
static std::vector<std::vector<bool> > ten_minutes_;

}

namespace mir {
namespace lsm {

const size_t ROWS = 1080;
const size_t COLS = 2160;

/*
From EMOSLIB:
     The 10 minute land sea mask file contains 1080 lines of 2160
     values with each value stored as one bit. (Each line of latitude
     is packed in 68*32 = 2176 bits). Each value consists
     of a 0 for sea or a 1 for land. The bits are assumed to be in
     the centre of a 10 minute square (e.g. the first value is
     assumed to be at 0  5' East and 89  55' North).

    ~mir/etc/ten-minutes.mask is a copy of ~emos/tables/interpolation/lsm_32_lsm10m01
*/

TenMinutesLSM::TenMinutesLSM(const std::string &name,
                             const param::MIRParametrisation &parametrisation,
                             const atlas::Grid &grid,
                             const std::string &which):
    Mask(name),
    path_("~mir/etc/ten-minutes.mask") {


    if (ten_minutes_.size() == 0) {

        eckit::Timer timer("Load 10 minutes LSM");
        eckit::AutoLock<eckit::Mutex> lock(local_mutex);
        eckit::Log::info() << "TenMinutesLSM loading " << path_ << std::endl;

        eckit::StdFile file(path_);
        ten_minutes_.resize(ROWS);

        size_t bytes = (((COLS + 31) / 32) * 32) / 8;
        unsigned char c;

        for (size_t i = 0; i < ROWS; i++) {
            size_t k = 0;
            std::vector<bool> &v = ten_minutes_[i] = std::vector<bool>(COLS);
            for (size_t j = 0; j < bytes ; j++) {
                ASSERT(fread(&c, 1, 1, file) == 1);
                for (size_t b = 0; b < 8 && k < COLS; b++) {
                    v[k++] = (c >> (7 - b)) & 0x1;
                }
            }
        }
#if 0
        eckit::StdFile pgm("10.pgm", "w");
        fwrite("P2\n", 3, 1, pgm);
        fwrite("2160\n1080\n", 10, 1, pgm);
        fwrite("1\n", 2, 1, pgm);

        for (size_t i = 0; i < ROWS; i++) {
            for (size_t j = 0; j < COLS; j++) {
                if (ten_minutes_[i][j])  {
                    fwrite("1\n", 2, 1, pgm);
                } else {
                    fwrite("0\n", 2, 1, pgm);
                }
            }
            fwrite("\n", 1, 1, pgm);
        }
#endif
    }

    eckit::Timer timer("Extract point from 10 minutes LSM");


    // NOTE: this is not using 3D coordinate systems

    std::vector<atlas::Grid::Point> points(grid.npts());
    grid.lonlat(points);

    mask_.reserve(points.size());

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

        int row = (90.0 - lat) * ROWS / 180;
        ASSERT(row >= 0 && row < ROWS);

        int col = lon * COLS / 360.0;
        ASSERT(col >= 0 && col < COLS);

        mask_.push_back(ten_minutes_[row][col]);
    }

}

TenMinutesLSM::~TenMinutesLSM() {
}

void TenMinutesLSM::hash(eckit::MD5 &md5) const {
    Mask::hash(md5);
    md5.add(path_.asString());
}

void TenMinutesLSM::print(std::ostream &out) const {
    out << "TenMinutesLSM[path=" << path_ << "]";
}


const std::vector<bool> &TenMinutesLSM::mask() const {
    return mask_;
}

//-----------------------------------------------------------------------------


}  // namespace logic
}  // namespace mir

