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

#include "eckit/linalg/SparseMatrix.h"
#include "eckit/types/FloatCompare.h"

#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/search/PointSearch.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Point2ToPoint3.h"
#include "mir/util/Trace.h"


namespace mir::method::gridbox {


namespace {


const MethodBuilder<GridBoxAverage2ndOrder> __builder("grid-box-average-2nd-order");


using closest_type       = std::vector<mir::search::PointSearch::PointValueType>;
using grid_box_dual_type = std::vector<size_t>;


grid_box_dual_type grid_box_dual(const util::GridBox& gbox, const GridBoxMethod::GridBoxes& boxes,
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

    auto closed = !neigh[N].empty() && !neigh[W].empty() && !neigh[S].empty() && !neigh[E].empty();

    std::vector<size_t> dual;
    dual.reserve(neigh[N].size() + neigh[W].size() + neigh[S].size() + neigh[E].size() + (closed ? 1 : 0));

    for (const auto& d : neigh[N].empty()   ? std::vector<Direction>{W, S, E}
                         : neigh[W].empty() ? std::vector<Direction>{S, E, N}
                         : neigh[S].empty() ? std::vector<Direction>{E, N, W}
                                            : std::vector<Direction>{N, W, S, E}) {
        dual.insert(dual.end(), neigh[d].cbegin(), neigh[d].cend());
        neigh[d].clear();
    }

    if (closed) {
        dual.insert(dual.end(), dual.front());
    }

    return dual;
}


// struct ExplodedMatrix {
//     ExplodedMatrix(const eckit::linalg::SparseMatrix& M) :
//         Nr(M.rows()), Nc(M.cols()), ia(Nr + 1, 0), ja(M.nnz()), a(M.nnz()) {
//         const auto base = static_cast<Index>(0);

//         for (size_t n = 0; n < M.nnz(); ++n) {
//             auto r = M.ia()[n] - base;
//             auto c = M.ja()[n] - base;
//             ASSERT(0 <= r && r < Nr);
//             ASSERT(0 <= c && c < Nc);

//             ia[r + 1]++;
//             ja[n] = c;
//         }
//     }

//     using Index  = eckit::linalg::Index;
//     using Scalar = eckit::linalg::Scalar;
//     using Size   = eckit::linalg::Size;

//     const Size Nr;
//     const Size Nc;
//     std::vector<Index> ia;
//     std::vector<Index> ja;
//     std::vector<Scalar> a;
// };


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


    std::vector<grid_box_dual_type> dual;
    {
        trace::ProgressTimer progress("GridBoxAverage2ndOrder: build dual mesh", in.numberOfPoints(),
                                      {"grid box", "grid boxes"});

        const GridBoxes boxes(in);
        const auto R = boxes.getLongestGridBoxDiagonal() * 1.1;  // slightly larger than the longest diagonal

        auto tree = std::make_unique<search::PointSearch>(parametrisation_, in);
        util::Point2ToPoint3 point3(in, poleDisplacement());
        closest_type closest;

        for (const std::unique_ptr<repres::Iterator> it(in.iterator()); it->next();) {
            if (++progress) {
                log << *tree << std::endl;
            }

            tree->closestWithinRadius(point3(*(*it)), R, closest);
            dual.emplace_back(grid_box_dual(boxes.at(it->index()), boxes, closest));
        }
    }


    NOTIMP;


    {
        trace::Timer time{"GridBoxAverage2ndOrder: assemble 2nd order"};
        for (size_t i = 0; i < W.rows(); ++i) {
            for (auto it = W.begin(i); it != W.end(i); ++it) {
                ASSERT(it.col() < W.cols());

                // TODO
            }
        }
    }
}


const char* GridBoxAverage2ndOrder::name() const {
    return "grid-box-average-2nd-order";
}


int GridBoxAverage2ndOrder::version() const {
    return 0;
}


}  // namespace mir::method::gridbox
