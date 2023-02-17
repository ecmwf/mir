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


#include "mir/action/plan/Job.h"

#include "mir/action/context/Context.h"
#include "mir/action/io/Copy.h"
#include "mir/action/plan/ActionPlan.h"
#include "mir/api/MIRJob.h"
#include "mir/input/MIRInput.h"
#include "mir/key/Key.h"
#include "mir/key/style/MIRStyle.h"
#include "mir/param/CombinedParametrisation.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"


namespace mir::action {


Job::Job(const api::MIRJob& job, input::MIRInput& input, output::MIROutput& output, bool compress) :
    input_(input), output_(output) {

    // get input and parameter-specific parametrisations
    static param::DefaultParametrisation defaults;
    const param::MIRParametrisation& metadata = input.parametrisation();

    combined_ = std::make_unique<param::CombinedParametrisation>(job, metadata, defaults);
    plan_     = std::make_unique<ActionPlan>(*combined_);


    // skip preparing an Action plan if nothing to do, or input is already what was specified
    if (!key::Key::postProcess(job) && job.matchAll(metadata)) {
        plan_->add(new io::Copy(*combined_, output_));
    }
    else {
        std::unique_ptr<key::style::MIRStyle> style(key::style::MIRStyleFactory::build(*combined_));
        style->prepare(*plan_, output_);

        if (compress) {
            plan_->compress();
        }
    }

    if (Log::debug_active()) {
        plan_->dump(Log::debug() << "Action plan is:"
                                    "\n");
    }

    ASSERT(plan_->ended());
}


Job::~Job() = default;


void Job::execute(util::MIRStatistics& statistics) const {
    ASSERT(plan_);

    context::Context ctx(input_, statistics);
    plan_->execute(ctx);
}


void Job::estimate(api::MIREstimation& estimation) const {
    ASSERT(plan_);

    util::MIRStatistics statistics;
    context::Context ctx(input_, statistics);
    plan_->estimate(ctx, estimation);
}

const ActionPlan& Job::plan() const {
    return *plan_;
}


const param::MIRParametrisation& Job::parametrisation() const {
    ASSERT(combined_);
    return *combined_;
}


}  // namespace mir::action
