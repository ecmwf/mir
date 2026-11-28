// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/util/GridBox.h"

#include <algorithm>
#include <cmath>
#include <ostream>

#include "eckit/geo/area/BoundingBox.h"
#include "eckit/types/FloatCompare.h"

#include "mir/util/Angles.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Types.h"


namespace mir::util {


GridBox::GridBox(double north, double west, double south, double east) :
    north_(north), west_(west), south_(south), east_(east) {
    ASSERT(SOUTH_POLE <= south_ && south_ <= north_ && north_ <= NORTH_POLE);
    ASSERT(west_ <= east_ && east_ <= west_ + LONGITUDE_GLOBE);
}


double GridBox::area() const {
    static const Earth EARTH;
    return EARTH.area(eckit::geo::area::BoundingBox{north_, west_, south_, east_});
}


double GridBox::diagonal() const {
    return Earth::distance(PointLonLat{west_, north_}, PointLonLat{east_, south_});
}


PointLonLat GridBox::centre() const {
    return {0.5 * (west_ + east_), 0.5 * (north_ + south_)};
}


bool GridBox::contains(const PointLonLat& p) const {
    if (!eckit::types::is_approximately_lesser_or_equal(south_, p.lat(), PointLonLat::EPS) ||
        !eckit::types::is_approximately_lesser_or_equal(p.lat(), north_, PointLonLat::EPS)) {
        return false;
    }

    // at the poles, every longitude is the same point
    return p.pole() ||
           eckit::types::is_approximately_lesser_or_equal(normalise_longitude(p.lon(), west_), east_, PointLonLat::EPS);
}


bool GridBox::intersects(GridBox& other) const {
    auto n = std::min(north_, other.north_);
    auto s = std::max(south_, other.south_);

    if (!eckit::types::is_strictly_greater(n, s, PointLonLat::EPS)) {
        return false;
    }

    auto intersect = [](const GridBox& a, const GridBox& b, double& w, double& e) {
        auto ref = normalise_longitude(b.west_, a.west_);
        auto w_  = std::max(a.west_, ref);
        auto e_  = std::min(a.east_, normalise_longitude(b.east_, ref));

        if (eckit::types::is_strictly_greater(e_, w_, PointLonLat::EPS)) {
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
