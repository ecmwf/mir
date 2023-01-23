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
#include "mir/api/MIRJob.h"
#include "mir/data/MIRField.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir {
namespace output {


RawOutput::RawOutput(double* values, size_t count, param::SimpleParametrisation& metadata) :
    values_(values), metadata_(metadata), count_(count), size_(0) {}


RawOutput::RawOutput(param::SimpleParametrisation& metadata) :
    values_(nullptr), metadata_(metadata), count_(0), size_(0) {}


size_t RawOutput::save(const param::MIRParametrisation& /*param*/, context::Context& ctx) {
    const auto& field = ctx.field();
    field.validate();


    // save metadata
    {
        Log::debug() << "RawOutput::save metadata" << std::endl;
        repres::RepresentationHandle repres(field.representation());

        // (a hack)
        api::MIRJob job;
        repres->fillJob(job);
        job.copyValuesTo(metadata_);

        if (field.hasMissing()) {
            metadata_.set("missing_value", field.missingValue());
        }
    }


    // save data
    ASSERT(field.dimensions() == 1);
    const auto& values = field.values(0);

    Log::debug() << "RawOutput::save values: " << values.size() << ", user: " << count_ << std::endl;

    size_ = values.size();
    ASSERT(size_ > 0);

    if (values_ == nullptr) {
        Log::debug() << "RawOutput::save allocating " << size_ << " values" << std::endl;
        values_ = new double[size_];
        count_  = size_;
    }

    ASSERT(size_ <= count_);
    std::memcpy(values_, values.data(), size_ * sizeof(double));

    return size_ * sizeof(double);
}


bool RawOutput::sameAs(const MIROutput& other) const {
    return dynamic_cast<const MIROutput*>(this) == &other;
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


}  // namespace output
}  // namespace mir
