// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/util/GridBox.h"

#include <algorithm>
#include <ostream>

#include "eckit/types/FloatCompare.h"

#include "mir/util/Atlas.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Latitude.h"
#include "mir/util/LongitudeDouble.h"


namespace mir::util {


GridBox::GridBox(double north, double west, double south, double east) :
    north_(north), west_(west), south_(south), east_(east) {
    ASSERT(Latitude::SOUTH_POLE.value() <= south_ && south_ <= north_ && north_ <= Latitude::NORTH_POLE.value());
    ASSERT(west_ <= east_ && east_ <= west_ + LongitudeDouble::GLOBE.value());
}


double GridBox::area() const {
    return Earth::area({west_, north_}, {east_, south_});
}


double GridBox::diagonal() const {
    return Earth::distance({west_, north_}, {east_, south_});
}


Point2 GridBox::centre() const {
    return {0.5 * (north_ + south_), 0.5 * (west_ + east_)};
}


bool GridBox::contains(const Point2& p) const {
    return eckit::types::is_approximately_lesser_or_equal(south_, p[0]) &&
           eckit::types::is_approximately_lesser_or_equal(p[0], north_) &&
           eckit::types::is_approximately_lesser_or_equal(LongitudeDouble(p[1]).normalise(west_).value(), east_);
}


bool GridBox::intersects(GridBox& other) const {
    auto n = std::min(north_, other.north_);
    auto s = std::max(south_, other.south_);

    if (!eckit::types::is_strictly_greater(n, s)) {
        return false;
    }

    auto intersect = [](const GridBox& a, const GridBox& b, double& w, double& e) {
        auto ref = LongitudeDouble(b.west_).normalise(a.west_).value();
        auto w_  = std::max(a.west_, ref);
        auto e_  = std::min(a.east_, LongitudeDouble(b.east_).normalise(ref).value());

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
