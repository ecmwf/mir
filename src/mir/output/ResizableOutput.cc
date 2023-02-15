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


#include "mir/output/ResizableOutput.h"

#include <ostream>

#include "mir/action/context/Context.h"
#include "mir/api/MIRJob.h"
#include "mir/data/MIRField.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::output {


ResizableOutput::ResizableOutput(std::vector<double>& values, param::SimpleParametrisation& metadata) :
    values_(values), metadata_(metadata) {}


size_t ResizableOutput::save(const param::MIRParametrisation& /*param*/, context::Context& ctx) {
    const auto& field = ctx.field();
    field.validate();


    // save metadata
    {
        Log::debug() << "ResizableOutput::save metadata" << std::endl;
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
    out << "ResizableOutput[size=" << size() << "]";
}


size_t ResizableOutput::size() const {
    return values_.size();
}


}  // namespace mir::output
