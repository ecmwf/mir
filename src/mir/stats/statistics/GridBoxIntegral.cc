// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/stats/statistics/GridBoxIntegral.h"

#include <limits>

#include "mir/data/MIRField.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/GridBox.h"


namespace mir::stats::statistics {


GridBoxIntegral::GridBoxIntegral(const param::MIRParametrisation& parametrisation) :
    Statistics(parametrisation), Counter(parametrisation), integral_(std::numeric_limits<double>::quiet_NaN()) {
    reset();
}


void GridBoxIntegral::reset() {
    integral_ = std::numeric_limits<double>::quiet_NaN();
}


void GridBoxIntegral::execute(const data::MIRField& field) {

    ASSERT(field.dimensions() == 1);
    const repres::RepresentationHandle rep(field.representation());
    ASSERT(rep);

    const auto boxes   = rep->gridBoxes();
    const auto& values = field.values(0);
    ASSERT(values.size() == rep->numberOfPoints());
    ASSERT(values.size() == boxes.size());

    integral_ = 0.;
    for (size_t i = 0; i < values.size(); ++i) {
        auto value = values[i];
        if (count(value)) {
            integral_ += boxes[i].area() * value;
        }
    }
}


void GridBoxIntegral::print(std::ostream& out) const {
    out << "GridBoxIntegral[" << integral_ << "]";
}


static const StatisticsBuilder<GridBoxIntegral> __stats("grid-box-integral");


}  // namespace mir::stats::statistics
