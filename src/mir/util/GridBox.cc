/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/util/GridBox.h"

#include <algorithm>
#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "eckit/types/FloatCompare.h"

#include "mir/api/Atlas.h"


namespace mir {
namespace util {


GridBox::GridBox(GridBox::LatitudeRange& lat, GridBox::LongitudeRange& lon) :
    north_(lat.north),
    west_(lon.west),
    south_(lat.south),
    east_(lon.east) {}


double GridBox::area() const {
    return atlas::util::Earth::area({west_, north_}, {east_, south_});
}


double GridBox::normalise(double lon, double minimum) {
    while (lon < minimum) {
        lon += GLOBE;
    }
    while (lon >= minimum + GLOBE) {
        lon -= GLOBE;
    }
    return lon;
}


bool GridBox::intersects(GridBox& other) const {
    double n = std::min(north_, other.north_);
    double s = std::max(south_, other.south_);

    bool intersectsSN = s <= n;
    if (!intersectsSN) {
        n = s;
    }

    auto intersect = [](const GridBox& a, const GridBox& b, double w, double e) {
        auto ref = normalise(b.west_, a.west_);
        auto w_  = std::max(a.west_, ref);
        auto e_  = std::min(a.east_, normalise(b.east_, ref));

        if (w_ <= e_) {
            w = w_;
            e = e_;
            return true;
        }
        return false;
    };

    auto w = std::min(west_, other.west_);
    auto e = w;

    bool intersectsWE = west_ <= other.west_ ? intersect(*this, other, w, e) || intersect(other, *this, w, e)
                                             : intersect(other, *this, w, e) || intersect(*this, other, w, e);

    ASSERT(w <= e);

    LatitudeRange sn(s, n);
    LongitudeRange we(w, e);
    other = {sn, we};

    return intersectsSN && intersectsWE;
}


void GridBox::print(std::ostream& out) const {
    out << "GridBox[north=" << north_ << ",west=" << west_ << ",south=" << south_ << ",east=" << east_ << "]";
}


GridBox::LongitudeRange::LongitudeRange(double _west, double _east) : west(_west), east(_east) {
    east = normalise(east, west);
    ASSERT(west <= east);
}


bool GridBox::LongitudeRange::intersects(const GridBox::LongitudeRange& other) const {

    auto intersect = [](const LongitudeRange& a, const LongitudeRange& b, double& w, double& e) {
        auto ref = normalise(b.west, a.west);
        auto w_  = std::max(a.west, ref);
        auto e_  = std::min(a.east, normalise(b.east, ref));

        if (w_ <= e_) {
            w = w_;
            e = e_;
            return eckit::types::is_strictly_greater(e, w);
        }
        return false;
    };

    auto w = std::min(west, other.west);
    auto e = w;

    return west <= other.west ? intersect(*this, other, w, e) || intersect(other, *this, w, e)
                              : intersect(other, *this, w, e) || intersect(*this, other, w, e);
}


double GridBox::LongitudeRange::normalise(double lon, double minimum) {
    while (lon < minimum) {
        lon += GLOBE;
    }
    while (lon >= minimum + GLOBE) {
        lon -= GLOBE;
    }
    return lon;
}


GridBox::LatitudeRange::LatitudeRange(double _south, double _north) : south(_south), north(_north) {
    ASSERT(south <= north);
}


bool GridBox::LatitudeRange::intersects(const GridBox::LatitudeRange& other) const {
    double n = std::min(north, other.north);
    double s = std::max(south, other.south);
    return eckit::types::is_strictly_greater(n, s);
}


}  // namespace util
}  // namespace mir
