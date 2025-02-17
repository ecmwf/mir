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
#include <map>
#include <memory>
#include <vector>

#include "eckit/linalg/SparseMatrix.h"
#include "eckit/linalg/allocator/StandardContainerAllocator.h"

#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Trace.h"


namespace mir::method::gridbox {


static const MethodBuilder<GridBoxAverage2ndOrder> __builder("grid-box-average-2nd-order");


GridBoxAverage2ndOrder::GridBoxAverage2ndOrder(const param::MIRParametrisation& param) :
    GridBoxMethod(param), firstOrder_(param) {}


void GridBoxAverage2ndOrder::assemble(util::MIRStatistics& stats, WeightMatrix& W, const repres::Representation& in,
                                      const repres::Representation& out) const {
    using allocator_t        = eckit::linalg::allocator::StandardContainerAllocator;
    using matrix_container_t = allocator_t::container_type;

    auto Nr = out.numberOfPoints();
    auto Nc = in.numberOfPoints();
    matrix_container_t mat(Nr);
    {
        trace::Timer time{"GridBoxAverage2ndOrder: assemble 1st order"};

        WeightMatrix M(Nr, Nc);
        firstOrder_.assemble(stats, M, in, out);
        ASSERT(M.rows() == Nr);

        for (auto it = M.begin(); it != M.end(); ++it) {
            mat.at(it.row())[it.col()] = *it;
        }
    }


    std::unique_ptr<const GridBoxes> inBoxes;
    {
        trace::Timer time("GridBoxAverage2ndOrder: input grid boxes (dual)");
        inBoxes = std::make_unique<GridBoxes>(in, true);
        ASSERT(inBoxes->size() == Nc);
    }


    {
        trace::ProgressTimer progress{"GridBoxAverage2ndOrder: assemble 2nd order", Nr, {"grid box", "grid boxes"}};

        // set input and output grid boxes
        const GridBoxes outBoxes(out);
        ASSERT(outBoxes.size() == Nr);

        for (size_t i = 0; i < Nr; ++i, ++progress) {
            const auto& box = outBoxes.at(i);
            auto& row       = mat[i];

            // calculate contributions from output intersecting with input grid box dual
            std::vector<matrix_container_t::value_type::key_type> indices;
            indices.reserve(row.size());
            std::for_each(row.begin(), row.end(), [&](const auto& ja) { indices.emplace_back(ja.first); });

            for (auto j : indices) {
                for (const auto& partition : inBoxes->at(j).dual().intersect(box)) {
                    auto a = partition.poly.area();
                    ASSERT(a > 0.);

                    row[partition.j] += a / 2.;  // FIXME not exactly this
                    row[partition.k] += a / 2.;

                    // FIXME
                    NOTIMP;
                }
            }
        }

        WeightMatrix M{new allocator_t{Nr, Nc, mat}};
        W.swap(M);
    }
}


const char* GridBoxAverage2ndOrder::name() const {
    return "grid-box-average-2nd-order";
}


int GridBoxAverage2ndOrder::version() const {
    return 0;
}


}  // namespace mir::method::gridbox
