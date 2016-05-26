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


#include <iostream>

#include "mir/action/ActionPlan.h"

#include "mir/data/MIRField.h"
#include "mir/input/MIRInput.h"
#include "mir/logic/MIRLogic.h"
#include "mir/output/MIROutput.h"
#include "mir/param/MIRCombinedParametrisation.h"
#include "mir/param/MIRDefaults.h"
#include "mir/log/MIR.h"

#include "mir/action/Job.h"
#include "mir/api/MIRJob.h"


namespace mir {
namespace action {



Job::Job(const api::MIRJob &job, input::MIRInput &input, output::MIROutput &output):
    job_(job),
    input_(input),
    output_(output)  {

    const param::MIRParametrisation &defaults = param::MIRDefaults::instance();
    const param::MIRParametrisation &metadata = input.parametrisation();

    combined_.reset(new param::MIRCombinedParametrisation(job, metadata, defaults));
    plan_.reset(new action::ActionPlan(*combined_));

    if (job.empty()) {
        eckit::Log::trace<MIR>() << "Nothing to do (no request)" << std::endl;
        return;
    }

    if (job.matches(metadata)) {
        eckit::Log::trace<MIR>() << "Nothing to do (field matches)" << std::endl;
        return;
    }


    eckit::ScopedPtr< logic::MIRLogic > logic(logic::MIRLogicFactory::build(*combined_));
    logic->prepare(*plan_);

    eckit::Log::trace<MIR>() << "Action plan is: " << *plan_ << std::endl;

}


Job::~Job() {
}


void Job::execute() const {

    if (plan_->empty()) {
        output_.copy(job_, input_);
        return;
    }

    eckit::ScopedPtr< data::MIRField > field(input_.field());
    eckit::Log::trace<MIR>() << "Field is " << *field << std::endl;

    plan_->execute(*field);

    output_.save(*combined_, input_, *field);
}


}  // namespace action
}  // namespace mir

