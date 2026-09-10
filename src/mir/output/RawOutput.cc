// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/output/RawOutput.h"

#include <cstring>
#include <memory>
#include <ostream>

#include "eckit/geo/Grid.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::output {


RawOutput::RawOutput(double* values, size_t count, param::SimpleParametrisation& metadata) :
    values_(values), count_(count), metadata_(metadata), size_(0) {}


size_t RawOutput::save(const param::MIRParametrisation& /*param*/, context::Context& ctx) {
    const auto& field = ctx.field();
    field.validate();

    // save metadata
    std::unique_ptr<const eckit::geo::Grid> grid(
        eckit::geo::GridFactory::build(repres::RepresentationHandle(field.representation())->spec()));
    metadata_.set("grid", grid->spec_str());
    if (field.hasMissing()) {
        metadata_.set("missing_value", field.missingValue());
    }

    // save data
    ASSERT(field.dimensions() == 1);
    const auto& values = field.values(0);

    size_ = values.size();
    ASSERT(size_ <= count_);
    std::memcpy(values_, values.data(), size_ * sizeof(double));

    return size_ * sizeof(double);
}


bool RawOutput::sameAs(const MIROutput& other) const {
    return this == &other;
}


bool RawOutput::sameParametrisation(const param::MIRParametrisation& /*unused*/,
                                    const param::MIRParametrisation& /*unused*/) const {
    return true;
}


bool RawOutput::printParametrisation(std::ostream& /*out*/, const param::MIRParametrisation& /*param*/) const {
    return false;
}


void RawOutput::print(std::ostream& out) const {
    out << "RawOutput[count=" << count_ << ", size=" << size_ << "]";
}


size_t RawOutput::size() const {
    return size_;
}


}  // namespace mir::output
