// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/output/ResizableOutput.h"

#include <memory>
#include <ostream>

#include "eckit/geo/Grid.h"
#include "eckit/spec/Spec.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"


namespace mir::output {


ResizableOutput::ResizableOutput(std::vector<double>& values, param::SimpleParametrisation& metadata) :
    values_(values), metadata_(metadata) {}


size_t ResizableOutput::save(const param::MIRParametrisation& /*param*/, context::Context& ctx) {
    const auto& field = ctx.field();
    field.validate();

    // save metadata
    std::unique_ptr<const eckit::geo::Grid> grid(
        eckit::geo::GridFactory::build(repres::RepresentationHandle(field.representation())->spec()));
    ASSERT(grid);

    metadata_.set("grid", grid->spec_str());
    if (field.hasMissing()) {
        metadata_.set("missing_value", field.missingValue());
    }

    // save data
    ASSERT(field.dimensions() == 1);
    ASSERT(field.values(0).size() == grid->size());
    values_ = field.values(0);

    return values_.size() * sizeof(double);
}


bool ResizableOutput::sameAs(const MIROutput& other) const {
    return dynamic_cast<const MIROutput*>(this) == &other;
}


bool ResizableOutput::sameParametrisation(const param::MIRParametrisation& /*unused*/,
                                          const param::MIRParametrisation& /*unused*/) const {
    return true;
}


bool ResizableOutput::printParametrisation(std::ostream& /*out*/, const param::MIRParametrisation& /*param*/) const {
    return false;
}


void ResizableOutput::print(std::ostream& out) const {
    out << "ResizableOutput[#values=" << size() << "]";
}


size_t ResizableOutput::size() const {
    return values_.size();
}


}  // namespace mir::output
