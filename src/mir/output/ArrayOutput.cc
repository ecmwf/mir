// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/output/ArrayOutput.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <numeric>
#include <ostream>

#include "eckit/geo/Grid.h"
#include "eckit/types/FloatCompare.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"


namespace mir::output {


size_t ArrayOutput::size() const {
    constexpr size_t ONE = 1;
    return std::accumulate(shape_.begin(), shape_.end(), ONE, [](size_t a, size_t b) { return a * b; });
}


size_t ArrayOutput::save(const param::MIRParametrisation&, context::Context& ctx) {
    const auto& field = ctx.field();
    field.validate();

    // save metadata
    std::unique_ptr<const eckit::geo::Grid> grid(
        eckit::geo::GridFactory::build(repres::RepresentationHandle(field.representation())->spec()));
    ASSERT(grid);

    shape_    = grid->shape();
    gridspec_ = grid->spec_str();

    // save data
    ASSERT(field.dimensions() == 1);
    ASSERT(field.values(0).size() == grid->size());
    values_ = field.values(0);

    if (auto mv = field.missingValue();
        std::any_of(values_.begin(), values_.end(), [mv, mv_isnan = std::isnan(mv)](auto v) {
            return mv_isnan ? std::isnan(v) : eckit::types::is_approximately_equal(v, mv);
        })) {
        missingValue_ = mv;
    }
    else {
        missingValue_ = std::numeric_limits<double>::quiet_NaN();
    }

    return values_.size() * sizeof(double);
}


void ArrayOutput::print(std::ostream& out) const {
    out << "ArrayOutput[#values=" << values_.size() << ",shape=[";
    const auto* sep = "";
    for (auto s : shape_) {
        out << sep << s;
        sep = ",";
    }
    out << "],gridspec=[" << gridspec_ << "]]";
}


}  // namespace mir::output
