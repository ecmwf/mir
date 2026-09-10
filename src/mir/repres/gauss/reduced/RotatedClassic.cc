// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/gauss/reduced/RotatedClassic.h"

#include <ostream>

#include "mir/util/Atlas.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"

#include "eckit/spec/Custom.h"


namespace mir::repres::gauss::reduced {


RotatedClassic::RotatedClassic(size_t N, const util::Rotation& rotation, const util::BoundingBox& bbox,
                               double angularPrecision) :
    Classic(N, bbox, angularPrecision), rotation_(rotation) {}


void RotatedClassic::print(std::ostream& out) const {
    out << "RotatedClassic["
           "N="
        << N_ << ",bbox=" << bbox_ << ",rotation=" << rotation_ << "]";
}


void RotatedClassic::makeName(std::ostream& out) const {
    Classic::makeName(out);
    rotation_.makeName(out);
}


bool RotatedClassic::sameAs(const Representation& /*unused*/) const {
    NOTIMP;
}


Iterator* RotatedClassic::iterator() const {
    return rotatedIterator(rotation_);
}


void RotatedClassic::fillGrib(grib_info& info) const {
    Classic::fillGrib(info);
    rotation_.fillGrib(info);
    info.grid.grid_type = CODES_UTIL_GRID_SPEC_REDUCED_ROTATED_GG;
}


void RotatedClassic::fillSpec(CustomSpec& spec) const {
    Classic::fillSpec(spec);
    rotation_.fillSpec(spec);
}


atlas::Grid RotatedClassic::atlasGrid() const {
    return rotate_atlas_grid(rotation_.rotation(), Classic::atlasGrid());
}


const Gridded* RotatedClassic::croppedRepresentation(const util::BoundingBox& bbox) const {
    return new RotatedClassic(N_, rotation_, bbox, angularPrecision_);
}


}  // namespace mir::repres::gauss::reduced
