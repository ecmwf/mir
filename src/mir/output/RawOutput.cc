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


#include "mir/output/RawOutput.h"

#include <cstring>
#include <ostream>

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir {
namespace output {


RawOutput::RawOutput(double* values, size_t count) : values_(values), count_(count), size_(0) {}


bool RawOutput::sameAs(const MIROutput& other) const {
    return this == &other;
}


bool RawOutput::sameParametrisation(const param::MIRParametrisation& /*unused*/,
                                    const param::MIRParametrisation& /*unused*/) const {
    NOTIMP;
}


bool RawOutput::printParametrisation(std::ostream& /*out*/, const param::MIRParametrisation& /*param*/) const {
    NOTIMP;
}


size_t RawOutput::save(const param::MIRParametrisation& /*param*/, context::Context& ctx) {
    const data::MIRField& field = ctx.field();

    field.validate();
    // field.hasMissing();
    // field.missingValue();


    ASSERT(field.dimensions() == 1);
    const MIRValuesVector& values = field.values(0);

    Log::debug() << "RawOutput::save values: " << values.size() << ", user: " << count_ << std::endl;

    size_ = values.size();
    ASSERT(size_ <= count_);
    std::memcpy(values_, &values[0], size_ * sizeof(double));

    return size_ * sizeof(double);
}


void RawOutput::print(std::ostream& out) const {
    out << "RawOutput[count=" << count_ << ", size=" << size_ << "]";
}


size_t RawOutput::size() const {
    return size_;
}


}  // namespace output
}  // namespace mir
