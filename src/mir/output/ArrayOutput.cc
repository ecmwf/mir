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


#include "mir/output/ArrayOutput.h"

#include <algorithm>
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
    values_       = field.values(0);

    auto mv       = field.missingValue();
    missingValue_ = std::any_of(values_.begin(), values_.end(),
                                [&mv](double v) { return eckit::types::is_approximately_equal(v, mv); })
                        ? mv
                        : std::numeric_limits<double>::quiet_NaN();

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
