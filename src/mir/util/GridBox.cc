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

#include "eckit/types/FloatCompare.h"

#include "mir/util/Atlas.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Types.h"


static constexpr double GLOBE      = 360.;
static constexpr double NORTH_POLE = 90.;
static constexpr double SOUTH_POLE = -90.;

double normalise(double lon, double minimum) {
    while (lon < minimum) {
        lon += GLOBE;
    }
    while (lon >= minimum + GLOBE) {
        lon -= GLOBE;
    }
    return lon;
}


namespace mir::util {


GridBox::GridBox(double north, double west, double south, double east) :
    north_(north), west_(west), south_(south), east_(east) {
    ASSERT(SOUTH_POLE <= south_ && south_ <= north_ && north_ <= NORTH_POLE);
    ASSERT(west_ <= east_ && east_ <= west_ + GLOBE);
}


double GridBox::area() const {
    return Earth::area({west_, north_}, {east_, south_});
}


double GridBox::diagonal() const {
    return Earth::distance({west_, north_}, {east_, south_});
}


bool GridBox::intersects(GridBox& other) const {
    double n = std::min(north_, other.north_);
    double s = std::max(south_, other.south_);

    if (!eckit::types::is_strictly_greater(n, s)) {
        return false;
    }

    auto intersect = [](const GridBox& a, const GridBox& b, double& w, double& e) {
        double ref = normalise(b.west_, a.west_);
        double w_  = std::max(a.west_, ref);
        double e_  = std::min(a.east_, normalise(b.east_, ref));

        if (eckit::types::is_strictly_greater(e_, w_)) {
            w = w_;
            e = e_;
            return true;
        }
        return false;
    };

    double w = std::min(west_, other.west_);
    double e = w;

    if (west_ <= other.west_ ? intersect(*this, other, w, e) || intersect(other, *this, w, e)
                             : intersect(other, *this, w, e) || intersect(*this, other, w, e)) {
        ASSERT(w <= e);
        other = {n, w, s, e};
        return true;
    }
    return false;
}


GridBox& GridBox::operator=(const GridBox& other) {
    north_ = other.north_;
    west_  = other.west_;
    south_ = other.south_;
    east_  = other.east_;
    return *this;
};


void GridBox::print(std::ostream& out) const {
    out << "GridBox[north=" << north_ << ",west=" << west_ << ",south=" << south_ << ",east=" << east_ << "]";
}


}  // namespace mir::util
