// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/gauss/reduced/RotatedFromPL.h"

#include <ostream>

#include "mir/util/Atlas.h"
#include "mir/util/Grib.h"

#include "eckit/spec/Custom.h"


namespace mir::repres::gauss::reduced {


RotatedFromPL::RotatedFromPL(const param::MIRParametrisation& parametrisation) :
    FromPL(parametrisation), rotation_(parametrisation) {}


RotatedFromPL::RotatedFromPL(size_t N, const std::vector<long>& pl, const util::Rotation& rotation,
                             const util::BoundingBox& bbox, double angularPrecision) :
    FromPL(N, pl, bbox, angularPrecision), rotation_(rotation) {}


void RotatedFromPL::print(std::ostream& out) const {
    out << "RotatedFromPL["
           "N="
        << N_ << ",bbox=" << bbox_ << ",rotation=" << rotation_ << "]";
}


void RotatedFromPL::makeName(std::ostream& out) const {
    FromPL::makeName(out);
    rotation_.makeName(out);
}


bool RotatedFromPL::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const RotatedFromPL*>(&other);
    return (o != nullptr) && (rotation_ == o->rotation_) && FromPL::sameAs(other);
}


Iterator* RotatedFromPL::iterator() const {
    return rotatedIterator(rotation_);
}


void RotatedFromPL::fillGrib(grib_info& info) const {
    FromPL::fillGrib(info);
    rotation_.fillGrib(info);
    info.grid.grid_type = CODES_UTIL_GRID_SPEC_REDUCED_ROTATED_GG;
}


void RotatedFromPL::fillSpec(CustomSpec& spec) const {
    FromPL::fillSpec(spec);
    rotation_.fillSpec(spec);
}


atlas::Grid RotatedFromPL::atlasGrid() const {
    return rotate_atlas_grid(rotation_.rotation(), FromPL::atlasGrid());
}


const Gridded* RotatedFromPL::croppedRepresentation(const util::BoundingBox& bbox) const {
    return new RotatedFromPL(N_, pls(), rotation_, bbox, angularPrecision_);
}


static const RepresentationBuilder<RotatedFromPL> rotatedFromPL("reduced_rotated_gg");


}  // namespace mir::repres::gauss::reduced
