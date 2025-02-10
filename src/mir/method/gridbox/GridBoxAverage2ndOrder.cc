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


#include "mir/method/gridbox/GridBoxAverage2ndOrder.h"

#include <algorithm>
#include <functional>
#include <map>
#include <memory>
#include <vector>

#include "eckit/linalg/SparseMatrix.h"
#include "eckit/types/FloatCompare.h"

#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/search/PointSearch.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Point2ToPoint3.h"
#include "mir/util/Polygon2.h"
#include "mir/util/Trace.h"


namespace mir::method::gridbox {


namespace {


const MethodBuilder<GridBoxAverage2ndOrder> __builder("grid-box-average-2nd-order");


using closest_type         = std::vector<search::PointSearch::PointValueType>;
using polygon_indices_type = std::vector<size_t>;


polygon_indices_type grid_box_dual(const util::GridBox& gbox, const GridBoxMethod::GridBoxes& boxes,
                                   const closest_type& closest_boxes, double epsilon = 1e-6) {
    enum Direction
    {
        N,
        W,
        S,
        E,
        ALL
    };

    std::vector<size_t> neigh[ALL];

    for (const auto& c : closest_boxes) {
        const auto& box = boxes.at(c.payload());

        if (gbox.west() < box.east() && box.west() < gbox.east()) {
            if (eckit::types::is_approximately_equal(gbox.north(), box.south(), epsilon)) {
                neigh[N].push_back(c.payload());
            }
            else if (eckit::types::is_approximately_equal(gbox.south(), box.north(), epsilon)) {
                neigh[S].push_back(c.payload());
            }
        }
        else if (gbox.south() < box.north() && box.south() < gbox.north()) {
            if (eckit::types::is_approximately_equal(gbox.west(), box.east(), epsilon)) {
                neigh[W].push_back(c.payload());
            }
            else if (eckit::types::is_approximately_equal(gbox.east(), box.west(), epsilon)) {
                neigh[E].push_back(c.payload());
            }
        }
    }

    ASSERT(!neigh[N].empty() || !neigh[S].empty());
    std::sort(neigh[N].begin(), neigh[N].end(), std::greater<>());
    std::sort(neigh[S].begin(), neigh[S].end());

    ASSERT(!neigh[W].empty() || !neigh[E].empty());
    ASSERT(neigh[W].size() <= 1);
    ASSERT(neigh[E].size() <= 1);

    std::vector<size_t> dual;
    dual.reserve(neigh[N].size() + neigh[W].size() + neigh[S].size() + neigh[E].size() + 1);

    for (const auto& d : neigh[N].empty()   ? std::vector<Direction>{W, S, E}
                         : neigh[W].empty() ? std::vector<Direction>{S, E, N}
                         : neigh[S].empty() ? std::vector<Direction>{E, N, W}
                                            : std::vector<Direction>{N, W, S, E}) {
        dual.insert(dual.end(), neigh[d].cbegin(), neigh[d].cend());
        neigh[d].clear();
    }

    auto closed = !(neigh[N].empty() || neigh[W].empty() || neigh[S].empty() || neigh[E].empty());
    dual.insert(dual.end(), closed ? dual.front() : closest_boxes.front().payload());

    return dual;
}


}  // namespace


GridBoxAverage2ndOrder::GridBoxAverage2ndOrder(const param::MIRParametrisation& param) :
    GridBoxMethod(param), firstOrder_(param) {}


void GridBoxAverage2ndOrder::assemble(util::MIRStatistics& stats, WeightMatrix& W, const repres::Representation& in,
                                      const repres::Representation& out) const {
    auto& log = Log::debug();


    {
        trace::Timer time{"GridBoxAverage2ndOrder: assemble 1st order"};
        firstOrder_.assemble(stats, W, in, out);
        ASSERT(!W.empty());
    }


    struct grad_type {
        const size_t j;
        const size_t k;
        const util::Polygon2 P;
    };


    std::vector<std::vector<grad_type>> grad;

    {
        trace::ProgressTimer progress("GridBoxAverage2ndOrder: source gradient", in.numberOfPoints(),
                                      {"grid box", "grid boxes"});

        const GridBoxes inBoxes(in);
        const auto R = inBoxes.getLongestGridBoxDiagonal() * 1.1;  // slightly larger than the longest diagonal

        auto tree = std::make_unique<search::PointSearch>(parametrisation_, in);
        util::Point2ToPoint3 point3(in, poleDisplacement());
        closest_type closest;

        std::vector<Point2> P;
        P.reserve(in.numberOfPoints());
        for (const std::unique_ptr<repres::Iterator> it(in.iterator()); it->next();) {
            P.emplace_back(it->pointRotated());
        }

        grad.reserve(in.numberOfPoints());

        for (const std::unique_ptr<repres::Iterator> it(in.iterator()); it->next();) {
            if (++progress) {
                log << *tree << std::endl;
            }

            const auto& box = inBoxes.at(it->index());
            const util::Polygon2 clipper{{box.north(), box.west()},
                                         {box.north(), box.east()},
                                         {box.south(), box.east()},
                                         {box.south(), box.west()}};

            tree->closestWithinRadius(point3(*(*it)), R, closest);
            auto dual = grid_box_dual(box, inBoxes, closest);
            ASSERT(dual.size() >= 3);

            auto& grad_i = grad.emplace_back();
            grad_i.reserve(dual.size());

            for (int j = 0, k = 1, n = static_cast<int>(dual.size()); j < n; ++j, k = (k + 1) % n) {
                util::Polygon2 tri{{P[dual[j]], it->pointRotated(), P[dual[k]]}};
                tri.clip(clipper);

                if (auto Ak = tri.area(); Ak > 0.) {
                    grad_i.emplace_back(grad_type{dual[j], dual[k], std::move(tri)});
                }
            }
            if (grad.size() >= 2)
                break;
        };
    }


    {
        trace::Timer time{"GridBoxAverage2ndOrder: assemble 2nd order"};

        std::vector<eckit::linalg::Triplet> triplets;
        for (eckit::linalg::Size i = 0; i < W.rows(); ++i) {
            std::map<eckit::linalg::Size, eckit::linalg::Scalar> row;
            for (auto it = W.begin(i); it != W.end(i); ++it) {
                row.emplace(it.col(), *it);
            }

            NOTIMP;

            for (const auto& [j, a] : row) {
                triplets.emplace_back(i, j, a);
            }
        }

        W.setFromTriplets({triplets.begin(), triplets.end()});
    }
}


const char* GridBoxAverage2ndOrder::name() const {
    return "grid-box-average-2nd-order";
}


int GridBoxAverage2ndOrder::version() const {
    return 0;
}


}  // namespace mir::method::gridbox
