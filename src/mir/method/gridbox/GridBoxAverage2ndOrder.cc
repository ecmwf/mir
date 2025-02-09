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

#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::method::gridbox {


static const MethodBuilder<GridBoxAverage2ndOrder> __builder("grid-box-average-2nd-order");


GridBoxAverage2ndOrder::GridBoxAverage2ndOrder(const param::MIRParametrisation& param) :
    GridBoxMethod(param), firstOrder_(param) {}


void GridBoxAverage2ndOrder::assemble(util::MIRStatistics& stats, WeightMatrix& W, const repres::Representation& in,
                                      const repres::Representation& out) const {
    auto& log = Log::debug();


    log << "GridBoxAverage2ndOrder::assemble 1st order..." << std::endl;

    firstOrder_.assemble(stats, W, in, out);
    ASSERT(!W.empty());

    log << "GridBoxAverage2ndOrder::assemble 1st order." << std::endl;


    log << "GridBoxAverage2ndOrder::assemble 2nd order..." << std::endl;
    for (size_t i = 0; i < W.rows(); ++i) {
        for (auto it = W.begin(i); it != W.end(i); ++it) {
            ASSERT(it.col() < W.cols());

            // TODO
        }
    }

    NOTIMP;
    log << "GridBoxAverage2ndOrder::assemble 2nd order." << std::endl;
}


const char* GridBoxAverage2ndOrder::name() const {
    return "grid-box-average-2nd-order";
}


int GridBoxAverage2ndOrder::version() const {
    return 0;
}


}  // namespace mir::method::gridbox
