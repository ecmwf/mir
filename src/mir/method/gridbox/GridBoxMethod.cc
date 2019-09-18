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
#include <map>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "eckit/log/ProgressTimer.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/api/Atlas.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Angles.h"
#include "mir/util/Pretty.h"


namespace mir {
namespace method {
namespace gridbox {


struct LongitudeRange {
    LongitudeRange(double _west, double _east) : west(_west), east(_east) {
        east = normalise(east, west);
        ASSERT(west <= east);
    }

    bool intersects(const LongitudeRange& other) const {

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

    bool intersects(const LatitudeRange& other) const {
        double n = std::min(north, other.north);
        double s = std::max(south, other.south);
        return eckit::types::is_strictly_greater(n, s);
    }

    double south;
    double north;
};


class GridBox {
public:
    GridBox(LatitudeRange& lat, LongitudeRange& lon) :
        north_(lat.north),
        west_(lon.west),
        south_(lat.south),
        east_(lon.east) {}

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

        LatitudeRange sn(s, n);
        LongitudeRange we(w, e);
        other = {sn, we};

        return intersectsSN && intersectsWE;
    }

private:
    double north_;
    double west_;
    double south_;
    double east_;
    static constexpr double GLOBE = 360.;

    void print(std::ostream& out) const {
        out << "GridBox["
            << "north=" << north_ << ",west=" << west_ << ",south=" << south_ << ",east=" << east_ << "]";
    }

    friend std::ostream& operator<<(std::ostream& s, const GridBox& p) {
        p.print(s);
        return s;
    }
};

std::vector<GridBox> boxes(LatitudeRange lat, std::vector<double> lonEdges) {
    ASSERT(lonEdges.size() > 1);

    std::vector<GridBox> boxes;
    boxes.reserve(lonEdges.size() - 1);
    for (size_t i = 0; i < lonEdges.size() - 1; ++i) {
        LongitudeRange lon{lonEdges[i], lonEdges[i + 1]};
        boxes.emplace_back(lat, lon);
    }

    return boxes;
}


GridBoxMethod::~GridBoxMethod() = default;


bool GridBoxMethod::sameAs(const Method& other) const {
    auto o = dynamic_cast<const GridBoxMethod*>(&other);
    return o && name() == o->name() && MethodWeighted::sameAs(*o);
}


void GridBoxMethod::assemble(util::MIRStatistics&, WeightMatrix& W, const repres::Representation& in,
                             const repres::Representation& out) const {
    eckit::Channel& log = eckit::Log::debug<LibMir>();
    log << "GridBoxMethod::assemble (input: " << in << ", output: " << out << ")" << std::endl;


    log << "GridBoxMethod: calculate grid box latitude intersections" << std::endl;

    // map intersections of output rows to input rows
    // (b, a) are (output, input) grid box row indices

    auto aLatEdges = in.calculateGridBoxLatitudeEdges();
    ASSERT(aLatEdges.size() > 1);

    auto bLatEdges = out.calculateGridBoxLatitudeEdges();
    ASSERT(bLatEdges.size() > 1);

    std::map<size_t, std::vector<size_t>> rowIntersect;

    for (size_t b = 0; b < bLatEdges.size() - 1; ++b) {
        LatitudeRange boxB(bLatEdges[b + 1], bLatEdges[b]);
        for (size_t a = 0; a < aLatEdges.size() - 1; ++a) {
            LatitudeRange boxA(aLatEdges[a + 1], aLatEdges[a]);
            if (boxB.intersects(boxA)) {
                rowIntersect[b].push_back(a);
            }
        }
    }


    log << "GridBoxMethod: calculate grid box longitude intersections (on " << util::Pretty(rowIntersect.size(), "row") << ")" << std::endl;
    std::vector<WeightMatrix::Triplet> triplets;

    // TODO: no intelligent search yet, just brute forcing the intersections
    // TODO: triplets, really? why not writing to the matrix directly?
    {
        eckit::ProgressTimer progress("Intersecting", rowIntersect.size(), "row", double(5), log);

        // setup output grid boxes on row
        for (auto& it : rowIntersect) {
            size_t b = it.first;
            const auto boxesB(boxes({bLatEdges[b + 1], bLatEdges[b]}, out.calculateGridBoxLongitudeEdges(b)));

            // setup input grid boxes on row, and calculate intersections
            for (size_t a : it.second) {
                const auto boxesA(boxes({aLatEdges[a + 1], aLatEdges[a]}, in.calculateGridBoxLongitudeEdges(a)));

                for (size_t colB = 0; colB < boxesB.size(); ++colB) {
                    auto& boxB = boxesB[colB];

                    for (size_t colA = 0; colA < boxesA.size(); ++colA) {
                        auto boxA  = boxesA[colA];  // to intersect (copy)
                        auto areaA = boxA.area();

                        if (boxB.intersects(boxA)) {
                            ASSERT(areaA > 0.);
                            triplets.emplace_back(WeightMatrix::Triplet(colB, colA, boxA.area() / areaA));
                        }
                    }
                }
            }

            ++progress;
        }
    }


    log << "Intersected " << util::Pretty(triplets.size(), "grid box", "grid boxes") << std::endl;


    // fill sparse matrix
    W.setFromTriplets(triplets);
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
