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


#include "mir/util/GridBox.h"

#include <algorithm>
#include <ostream>

#include "eckit/geo/area/BoundingBox.h"
#include "eckit/types/FloatCompare.h"

#include "mir/util/Earth.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Types.h"


namespace mir::util {


static const Earth EARTH;


GridBox::GridBox(double north, double west, double south, double east) :
    north_(north), west_(west), south_(south), east_(east) {
    ASSERT(-PointLonLat::RIGHT_ANGLE <= south_ && south_ <= north_ && north_ <= PointLonLat::RIGHT_ANGLE);
    ASSERT(west_ <= east_ && east_ <= west_ + PointLonLat::FULL_ANGLE);
}


double GridBox::area() const {
    return EARTH.area(eckit::geo::area::BoundingBox{north_, west_, south_, east_});
}


double GridBox::diagonal() const {
    return EARTH.distance({west_, north_}, {east_, south_});
}


PointLonLat GridBox::centre() const {
    return {0.5 * (north_ + south_), 0.5 * (west_ + east_)};
}


bool GridBox::contains(const PointLonLat& p) const {
    return eckit::types::is_approximately_lesser_or_equal(south_, p.lat) &&
           eckit::types::is_approximately_lesser_or_equal(p.lat, north_) &&
           eckit::types::is_approximately_lesser_or_equal(PointLonLat::normalise_angle_to_minimum(p.lon, west_), east_);
}


bool GridBox::intersects(GridBox& other) const {
    auto n = std::min(north_, other.north_);
    auto s = std::max(south_, other.south_);

    if (!eckit::types::is_strictly_greater(n, s)) {
        return false;
    }

    auto intersect = [](const GridBox& a, const GridBox& b, double& w, double& e) {
        auto ref = PointLonLat::normalise_angle_to_minimum(b.west_, a.west_);
        auto w_  = std::max(a.west_, ref);
        auto e_  = std::min(a.east_, PointLonLat::normalise_angle_to_minimum(b.east_, ref));

        if (eckit::types::is_strictly_greater(e_, w_)) {
            w = w_;
            e = e_;
            return true;
        }
        return false;
    };

    auto w = std::min(west_, other.west_);
    auto e = w;

    if (west_ <= other.west_ ? intersect(*this, other, w, e) || intersect(other, *this, w, e)
                             : intersect(other, *this, w, e) || intersect(*this, other, w, e)) {
        ASSERT(w <= e);
        other = {n, w, s, e};
        return true;
    }
    return false;
}


void GridBox::print(std::ostream& out) const {
    out << "GridBox[north=" << north_ << ",west=" << west_ << ",south=" << south_ << ",east=" << east_ << "]";
}


}  // namespace mir::util
