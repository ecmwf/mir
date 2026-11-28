// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/data/CartesianVector2DField.h"

#include <cmath>
#include <memory>
#include <ostream>
#include <variant>

#include "eckit/geo/Grid.h"

#include "mir/repres/Representation.h"
#include "mir/util/Angles.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Rotation.h"
#include "mir/util/Types.h"


namespace mir::data {


namespace {

inline double normalise(double x) {
    return std::max(std::min(x, 1.0), -1.0);
}

}  // namespace


CartesianVector2DField::CartesianVector2DField(const repres::Representation* representation, bool /*hasMissing*/,
                                               double missingValue) :
    missingValue_(missingValue), representation_(representation) {

    ASSERT(representation_);
    representation_->attach();
}


CartesianVector2DField::~CartesianVector2DField() {
    ASSERT(representation_);
    representation_->detach();
}


void CartesianVector2DField::rotate(const util::Rotation& rotation, MIRValuesVector& valuesX,
                                    MIRValuesVector& valuesY) const {
    // setup results vectors
    ASSERT(!valuesX.empty());
    ASSERT(valuesX.size() == valuesY.size());
    // const size_t N = valuesX.size();

    // determine angle between meridians (c) using the (first) spherical law of cosines:
    // https://en.wikipedia.org/wiki/Spherical_law_of_cosines
    // NOTE: uses spherical (not geodetic) coordinates: C = θ = π / 2 - latitude
    ASSERT(rotation.south_pole_rotation_angle() == 0.);  // For now
    const double C     = util::degree_to_radian(90. - rotation.south_pole_latitude().value());
    const double cos_C = std::cos(C);
    const double sin_C = std::sin(C);

    // the same points, rotated and of the grid frame (unrotated)
    const auto& rotated   = representation_->canonicalGrid();
    const auto& unrotated = representation_->canonicalGridUnrotated();
    ASSERT(rotated.size() == unrotated.size());

    auto it = rotated.cbegin();
    for (auto jt = unrotated.cbegin(); it; ++it, ++jt) {
        auto& vx = valuesX.at(it.index());
        auto& vy = valuesY.at(it.index());

        if (vx == missingValue_ || vy == missingValue_) {
            vx = vy = missingValue_;
            continue;
        }

        const double lon_rotated = util::normalise_longitude(
            rotation.south_pole_longitude().value() - std::get<PointLonLat>(*it).lon(), -LONGITUDE_GLOBE / 2.);
        const double lon_unrotated = std::get<PointLonLat>(*jt).lon();

        const double a = util::degree_to_radian(lon_rotated);
        const double b = util::degree_to_radian(lon_unrotated);
        const double q = (sin_C * lon_rotated < 0.) ? 1. : -1.;  // correct quadrant

        const double cos_c = normalise(std::cos(a) * std::cos(b) + std::sin(a) * std::sin(b) * cos_C);
        const double sin_c = q * std::sqrt(1. - cos_c * cos_c);

        // TODO: use matrix multiplication
        const double x = cos_c * vx - sin_c * vy;
        const double y = sin_c * vx + cos_c * vy;
        vx             = x;
        vy             = y;
    }
}


void CartesianVector2DField::print(std::ostream& out) const {
    out << "CartesianVector2DField["
        << "]";
}


}  // namespace mir::data
