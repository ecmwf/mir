/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include <istream>

#include "mir/output/RawOutput.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/io/Length.h"

#include "mir/data/MIRField.h"
#include "mir/repres/Representation.h"
#include "mir/input/RawInput.h"
#include "mir/log/MIR.h"


namespace mir {
namespace output {


RawOutput::RawOutput(double *values, size_t count):
    values_(values),
    count_(count),
    size_(0),
    representation_(0) {
}


RawOutput::~RawOutput() {
    if (representation_) {
        representation_->detach();
    }
}


void RawOutput::copy(const param::MIRParametrisation &param, input::MIRInput &input) {
    NOTIMP;
    size_ = input.copy(values_, count_);
}


void RawOutput::save(const param::MIRParametrisation &param, input::MIRInput &input, data::MIRField &field) {
    field.validate();
    // field.hasMissing();
    // field.missingValue();


    ASSERT(field.dimensions() == 1);
    const std::vector<double> &values = field.values(0);

    eckit::Log::trace<MIR>() << "RawOutput::save values: " << values.size() << ", user: " << count_ << std::endl;

    size_ = values.size();
    ASSERT(size_ <= count_);
    ::memcpy(values_, &values[0], size_ * sizeof(double));

    // Use for HIRLAM like routines. Remove when emoslib compatibility not needed anymore
    ASSERT(!representation_);
    representation_ = field.representation();
    representation_->attach();

}

void RawOutput::print(std::ostream &out) const {
    out << "RawOutput[count=" << count_ << "]";
}

// Use for HIRLAM like routines. Remove when emoslib compatibility not needed anymore
void RawOutput::shape(size_t &ni, size_t &nj) const {
    ASSERT(representation_);
    representation_->shape(ni, nj);
}

size_t RawOutput::size() const {
    return size_;
}

eckit::Length RawOutput::total() const {
    return size_;
}


}  // namespace output
}  // namespace mir

