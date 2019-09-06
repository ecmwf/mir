/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/gridbox/GridBoxMethod.h"

#include <algorithm>
#include <cmath>
#include <iterator>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/api/Atlas.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Angles.h"


namespace mir {
namespace method {
namespace gridbox {


struct LongitudeRange {

    LongitudeRange(double _west, double _east) : west(_west), east(_east) {
        if (!eckit::types::is_approximately_equal(west, east)) {
            double eastNormalised = normalise(east, west);
            if (eckit::types::is_approximately_equal(eastNormalised, west)) {
                eastNormalised += GLOBE;
            }
            east = eastNormalised;
        }

        ASSERT(west <= east);
        ASSERT(east <= west + GLOBE);
    }

    double fraction(const LongitudeRange& other) const {

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

        bool intersects = west <= other.west ? intersect(*this, other, w, e) || intersect(other, *this, w, e)
                                             : intersect(other, *this, w, e) || intersect(*this, other, w, e);

        return intersects ? (e - w) / (east - west) : 0.;
    }

private:
    static double normalise(double lon, double minimum) {
        while (lon < minimum) {
            lon += GLOBE;
        }
        while (lon >= minimum + GLOBE) {
            lon -= GLOBE;
        }
        return lon;
    }

    double west;
    double east;
    static constexpr double GLOBE = 360.;
};


struct LatitudeRange {

    LatitudeRange(double _south, double _north) : south(_south), north(_north) { ASSERT(south <= north); }

    double fraction(const LatitudeRange& other) const {
        double n = std::min(north, other.north);
        double s = std::max(south, other.south);
        return eckit::types::is_strictly_greater(n, s) ? (n - s) / (north - south) : 0.;
    }

private:
    double south;
    double north;
};


class GridBox {
public:
    GridBox(double north, double west, double south, double east) :
        north_(north),
        west_(west),
        south_(south),
        east_(east) {
        normalise();
    }

    GridBox(size_t i, size_t j, const std::vector<double>& latitudeEdges, const std::vector<double>& longitudeEdges) {
        ASSERT(j + 1 < latitudeEdges.size());
        ASSERT(i + 1 < longitudeEdges.size());

        west_  = longitudeEdges[i];
        north_ = latitudeEdges[j];
        east_  = longitudeEdges[i + 1];
        south_ = latitudeEdges[j + 1];

        normalise();
    }

    double area() const { return atlas::util::Earth::area({west_, north_}, {east_, south_}); }

    static double normalise(double lon, double minimum) {
        while (lon < minimum) {
            lon += GLOBE;
        }
        while (lon >= minimum + GLOBE) {
            lon -= GLOBE;
        }
        return lon;
    }

    bool intersects(GridBox& other) const {

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
        other = {n, w, s, e};

        return intersectsSN && intersectsWE;
    }

private:
    double north_;
    double west_;
    double south_;
    double east_;
    static constexpr double GLOBE = 360.;

    void normalise() {
        if (!eckit::types::is_approximately_equal(west_, east_)) {
            double eastNormalised = normalise(east_, west_);
            if (eckit::types::is_approximately_equal(eastNormalised, west_)) {
                eastNormalised += GLOBE;
            }
            east_ = eastNormalised;
        }

        ASSERT(west_ <= east_);
        ASSERT(east_ <= west_ + GLOBE);
        ASSERT(north_ >= south_);
    }

    void print(std::ostream& out) const {
        out << "GridBox["
            << "north=" << north_ << ",west=" << west_ << ",south=" << south_ << ",east=" << east_ << "]";
    }

    friend std::ostream& operator<<(std::ostream& s, const GridBox& p) {
        p.print(s);
        return s;
    }
};

struct ij_t {
    size_t i;
    size_t j;
};


ij_t overlap_decreasing(const std::vector<double>& v, double value) {
    ASSERT(v.size() >= 2);
    ASSERT(v[1] < v[0]);

    auto less = [](double e1, double e2) { return !(e1 < e2); };
    auto i    = std::lower_bound(v.begin(), v.end(), value, less);
    ASSERT(i != v.begin());

    auto j(i);
    j++;
    //    if (*j)

    return {size_t(distance(v.begin(), i) - 1), 0};
}


size_t above_increasing(const std::vector<double>& v, double value) {
    ASSERT(v.size() >= 2);
    ASSERT(v[0] < v[1]);

    auto best = std::lower_bound(v.begin(), v.end(), value, [](double e1, double e2) { return !(e1 < e2); });
    ASSERT(best != v.begin());

    return size_t(distance(v.begin(), best) - 1);
}


GridBoxMethod::~GridBoxMethod() = default;


bool GridBoxMethod::sameAs(const Method& other) const {
    auto o = dynamic_cast<const GridBoxMethod*>(&other);
    return o && name() == o->name() && MethodWeighted::sameAs(*o);
}


void GridBoxMethod::assemble(util::MIRStatistics& statistics, WeightMatrix& W, const repres::Representation& in,
                             const repres::Representation& out) const {
    eckit::Channel& log = eckit::Log::debug<LibMir>();
    log << "GridBoxMethod::assemble (input: " << in << ", output: " << out << ")" << std::endl;
    NOTIMP;


    // 1) Calculate area fraction overlaps between the input and output representation points (as boxes)

    // TODO

    //    WeightMatrix M(out.numberOfPoints(), in.numberOfPoints());
    //    std::vector<WeightMatrix::Triplet> triplets;
    //    M.setFromTriplets(triplets);
    //    M.swap(W);
}


void GridBoxMethod::hash(eckit::MD5& md5) const {
    MethodWeighted::hash(md5);
    md5.add(name());
}


void GridBoxMethod::print(std::ostream& out) const {
    out << "GridBoxMethod["
        << "name=" << name() << ",";
    MethodWeighted::print(out);
    out << "]";
}


}  // namespace gridbox
}  // namespace method
}  // namespace mir
