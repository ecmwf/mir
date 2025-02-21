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


#include "mir/data/CartesianVector2DField.h"

#include <cmath>
#include <memory>
#include <ostream>

#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Angles.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Rotation.h"


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
    ASSERT(rotation.angle() == 0.);  // For now
    const double C     = util::degree_to_radian(90. - rotation.southPole().lat);
    const double cos_C = std::cos(C);
    const double sin_C = std::sin(C);

    for (const std::unique_ptr<repres::Iterator> it(representation_->iterator()); it->next();) {
        auto& vx = valuesX.at(it->index());
        auto& vy = valuesY.at(it->index());

        if (vx == missingValue_ || vy == missingValue_) {
            vx = vy = missingValue_;
            continue;
        }

        const LongitudeDouble lonRotated = rotation.southPole().lon - (*(*it))[1];
        const double lon_rotated         = lonRotated.normalise(LongitudeDouble::MINUS_DATE_LINE).value();
        const double lon_unrotated       = it->pointUnrotated().lon().value();

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
