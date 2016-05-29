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

#include "mir/action/plan/ActionPlan.h"
#include "mir/action/io/Save.h"
#include "mir/action/io/Copy.h"

#include "mir/data/MIRField.h"
#include "mir/input/MIRInput.h"
#include "mir/logic/MIRLogic.h"
#include "mir/output/MIROutput.h"
#include "mir/param/MIRCombinedParametrisation.h"
#include "mir/param/MIRDefaults.h"
#include "mir/log/MIR.h"

#include "mir/action/plan/Job.h"
#include "mir/api/MIRJob.h"


namespace mir {
namespace action {


Job::Job(const api::MIRJob &job, input::MIRInput &input, output::MIROutput &output):
    input_(input),
    output_(output)  {

    const param::MIRParametrisation &defaults = param::MIRDefaults::instance();
    const param::MIRParametrisation &metadata = input.parametrisation();

    combined_.reset(new param::MIRCombinedParametrisation(job, metadata, defaults));
    plan_.reset(new action::ActionPlan(*combined_));

    if (!job.empty() && !job.matches(metadata)) {

        eckit::ScopedPtr< logic::MIRLogic > logic(logic::MIRLogicFactory::build(*combined_));
        logic->prepare(*plan_);
    }

    if (plan_->empty()) {
        plan_->add(new action::Copy(*combined_, input_, output_));
    } else {
        plan_->add(new action::Save(*combined_, input_, output_));
    }

    eckit::Log::trace<MIR>() << "Action plan is: " << *plan_ << std::endl;
}

Job::~Job() {
}

void Job::execute(util::MIRStatistics& statistics) const {

    // This is an optimistation for MARS
    // We avoid to decode the input field
    if(plan_->size() == 1 && !plan_->action(0).needField()) {
        data::MIRField dummy(*combined_);
        plan_->execute(dummy, statistics);
        return;
    }

    eckit::ScopedPtr< data::MIRField > field(input_.field());
    eckit::Log::trace<MIR>() << "Field is " << *field << std::endl;
    plan_->execute(*field, statistics);
}


const ActionPlan& Job::plan() const {
    return *plan_;
}

}  // namespace action
}  // namespace mir

