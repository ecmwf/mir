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
#include <cmath>
#include <ostream>

#include "eckit/types/FloatCompare.h"

#include "mir/util/Atlas.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Latitude.h"
#include "mir/util/LongitudeDouble.h"


namespace mir::util {


GridBox::Dual::Dual(const GridBox& box, const DualIndices& indices, const DualPolygon& polygon) :
    proj(box.centre()), C(box.centre()) {
    ASSERT(indices.size() == polygon.size());
    reserve(indices.size());

    DualPolygon box_s{proj.to_stereo({box.north(), box.west()}), proj.to_stereo({box.south(), box.west()}),
                      proj.to_stereo({box.south(), box.east()}), proj.to_stereo({box.north(), box.east()})};
    box_s.simplify();

    for (int j = 0, k = 1, N = static_cast<int>(indices.size()); j < N; ++j, k = (k + 1) % N) {
        Polygon2 clipper{{0., 0.}, proj.to_stereo(polygon[j]), proj.to_stereo(polygon[k])};
        clipper.simplify();

        auto poly = box_s;
        poly.clip(clipper);

        if (!poly.empty()) {
            emplace_back(std::move(poly), indices[j], indices[k]);
        }
    }
}


std::vector<GridBox::DualPartition> GridBox::Dual::intersect(const GridBox& box) const {
    std::vector<GridBox::DualPartition> r;
    r.reserve(size());

    // calculate in local (stereographic) coordinates, return in (lat, lon) coordinates
    DualPolygon box_s{proj.to_stereo({box.north(), box.west()}), proj.to_stereo({box.south(), box.west()}),
                      proj.to_stereo({box.south(), box.east()}), proj.to_stereo({box.north(), box.east()})};
    box_s.simplify();

    for (const auto& p : *this) {
        auto dual_s = p.poly;
        dual_s.clip(box_s);

        if (!dual_s.empty()) {
            Polygon2 dual(dual_s);
            std::for_each(dual.begin(), dual.end(), [this](auto& p) { p = proj.to_latlon(p); });
            r.emplace_back(std::move(dual_s), p.j, p.k);
        }
    }

    return r;
}


GridBox::Dual::LatLonToStereographic::LatLonToStereographic(Point2 C) :
    R([](double latr, double lonr) {
        return eckit::maths::Matrix3<double>{std::sin(lonr),
                                             -std::cos(lonr),
                                             0.,
                                             std::cos(lonr) * std::sin(latr),
                                             std::sin(lonr) * std::sin(latr),
                                             -std::cos(latr),
                                             std::cos(lonr) * std::cos(latr),
                                             std::sin(lonr) * std::cos(latr),
                                             std::sin(latr)};
    }(degree_to_radian(C[0]), degree_to_radian(C[1]))),
    Ri([](double latr, double lonr) -> eckit::maths::Matrix3<double> {
        return eckit::maths::Matrix3<double>{std::sin(lonr),
                                             std::cos(lonr) * std::sin(latr),
                                             std::cos(lonr) * std::cos(latr),
                                             -std::cos(lonr),
                                             std::sin(lonr) * std::sin(latr),
                                             std::sin(lonr) * std::cos(latr),
                                             0.,
                                             -std::cos(latr),
                                             std::sin(latr)};
    }(degree_to_radian(C[0]), degree_to_radian(C[1]))) {}


Point2 GridBox::Dual::LatLonToStereographic::to_stereo(Point2 P) const {
    Point3 P3;
    Earth::convertSphericalToCartesian({P[1], P[0]}, P3);
    auto S = R * eckit::geo::Point3{P3[0], P3[1], P3[2]};
    return {S[0] / (1. + S[2]), S[1] / (1. + S[2])};
}


Point2 GridBox::Dual::LatLonToStereographic::to_latlon(Point2 S) const {
    const auto d = 1. / (1. + S[0] * S[0] + S[1] * S[1]);
    const auto Q = Ri * eckit::geo::Point3{2. * S[0] * d, 2. * S[1] * d, (-1. + S[0] * S[0] + S[1] * S[1]) * d};

    Point2 P;
    Earth::convertCartesianToSpherical({Q[0], Q[1], Q[2]}, P);
    return {P[1], P[0]};
}


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


void GridBox::dual(DualIndices&& indices, DualPolygon&& polygon) {
    ASSERT(!indices.empty());
    ASSERT(indices.size() == polygon.size());

    dual_.reset(new Dual{*this, std::move(indices), std::move(polygon)});
}


const GridBox::Dual& GridBox::dual() const {
    ASSERT(dual_);
    return *dual_;
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
