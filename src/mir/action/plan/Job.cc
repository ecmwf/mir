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

#include "mir/action/context/Context.h"
#include "mir/input/MIRInput.h"
#include "mir/style/MIRStyle.h"
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

    const param::MIRParametrisation &metadata = input.parametrisation();
    const param::MIRParametrisation &defaults = param::MIRDefaults::instance();

    combined_.reset(new param::MIRCombinedParametrisation(job, metadata, defaults));

    if (job.empty() || job.matches(metadata)) {
        plan_.reset(new action::ActionPlan(job));
        plan_->add(new action::Copy(job, output_));
        return;
    }

    plan_.reset(new action::ActionPlan(*combined_));

    eckit::ScopedPtr< style::MIRStyle > style(style::MIRStyleFactory::build(*combined_));
    style->prepare(*plan_);


    if (plan_->empty()) {
        plan_->add(new action::Copy(*combined_, output_));
    } else {
        plan_->add(new action::Save(*combined_, input_, output_));
    }

    eckit::Log::trace<MIR>() << "Action plan is: " << *plan_ << std::endl;
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

