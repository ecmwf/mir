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


#include "mir/action/plan/Job.h"

#include <iostream>
#include "mir/action/context/Context.h"
#include "mir/action/io/Copy.h"
#include "mir/action/io/Save.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/api/MIRJob.h"
#include "mir/config/LibMir.h"
#include "mir/input/MIRInput.h"
#include "mir/output/MIROutput.h"
#include "mir/param/MIRCombinedParametrisation.h"
#include "mir/style/MIRStyle.h"
#include "mir/param/DefaultParametrisation.h"


namespace mir {
namespace action {


Job::Job(const api::MIRJob& job, input::MIRInput& input, output::MIROutput& output) :
    input_(input),
    output_(output)  {

    // get input and parameter-specific parametrisations
    static param::DefaultParametrisation defaults;
    const param::MIRParametrisation& metadata = input.parametrisation();
    combined_.reset(new param::MIRCombinedParametrisation(job, metadata, defaults));

    eckit::ScopedPtr< style::MIRStyle > style(style::MIRStyleFactory::build(*combined_));


    // skip preparing an Action plan if nothing to do, or
    // input is already what was specified

    if (job.empty() || (!style->forcedPrepare(job, metadata))) {
        plan_.reset(new action::ActionPlan(job));
        plan_->add(new action::Copy(job, output_));
        return;
    }


    plan_.reset(new action::ActionPlan(*combined_));
    style->prepare(*plan_);


    if (plan_->empty()) {
        plan_->add(new action::Copy(*combined_, output_));
    } else {
        plan_->add(new action::Save(*combined_, input_, output_));
    }

    if(eckit::Log::debug<LibMir>()){
        // eckit::Log::debug<LibMir>() << "Action plan is: " << *plan_ << std::endl;
        eckit::Log::debug<LibMir>() << "Action plan is: " << std::endl;
        plan_->dump(eckit::Log::debug<LibMir>());
    }

}


Job::~Job() {
}


void Job::execute(util::MIRStatistics &statistics) const {

    ASSERT(plan_);
    context::Context ctx(input_, statistics);
    plan_->execute(ctx);
}


const ActionPlan &Job::plan() const {
    return *plan_;
}


const param::MIRParametrisation& Job::parametrisation() const {
    ASSERT(combined_);
    return *combined_;
}


}  // namespace action
}  // namespace mir

