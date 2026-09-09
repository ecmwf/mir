// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/gauss/reduced/RotatedOctahedral.h"

#include <ostream>

#include "mir/util/Atlas.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"

#include "eckit/spec/Custom.h"


namespace mir::repres::gauss::reduced {


RotatedOctahedral::RotatedOctahedral(size_t N, const util::Rotation& rotation, const util::BoundingBox& bbox,
                                     double angularPrecision) :
    Octahedral(N, bbox, angularPrecision), rotation_(rotation) {}


void RotatedOctahedral::print(std::ostream& out) const {
    out << "RotatedOctahedral["
           "N="
        << N_ << ",bbox=" << bbox_ << ",rotation=" << rotation_ << "]";
}


void RotatedOctahedral::makeName(std::ostream& out) const {
    Octahedral::makeName(out);
    rotation_.makeName(out);
}


bool RotatedOctahedral::sameAs(const Representation& /*unused*/) const {
    NOTIMP;
}


Iterator* RotatedOctahedral::iterator() const {
    return rotatedIterator(rotation_);
}


void RotatedOctahedral::fillGrib(grib_info& info) const {
    Octahedral::fillGrib(info);
    rotation_.fillGrib(info);
    info.grid.grid_type = CODES_UTIL_GRID_SPEC_REDUCED_ROTATED_GG;
}


void RotatedOctahedral::fillSpec(CustomSpec& spec) const {
    Octahedral::fillSpec(spec);
    rotation_.fillSpec(spec);
}


atlas::Grid RotatedOctahedral::atlasGrid() const {
    return rotate_atlas_grid(rotation_.rotation(), Octahedral::atlasGrid());
}


const Gridded* RotatedOctahedral::croppedRepresentation(const util::BoundingBox& bbox) const {
    return new RotatedOctahedral(N_, rotation_, bbox, angularPrecision_);
}


}  // namespace mir::repres::gauss::reduced
