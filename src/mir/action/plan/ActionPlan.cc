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


#include "mir/action/plan/ActionPlan.h"

#include <fstream>
#include <iostream>
#include <sstream>

#include "eckit/log/JSON.h"

#include "mir/action/context/Context.h"
#include "mir/action/plan/Action.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"


namespace mir {
namespace action {


ActionPlan::ActionPlan(const param::MIRParametrisation& parametrisation) : parametrisation_(parametrisation) {
    parametrisation_.userParametrisation().get("dump-plan-file", dumpPlanFile_);
    parametrisation_.userParametrisation().get("dump-statistics-file", dumpStatisticsFile_);
}


ActionPlan::~ActionPlan() {
    for (auto& p : *this) {
        delete p;
    }

    for (auto& p : runtimes_) {
        delete p;
    }
}


void ActionPlan::add(const std::string& name) {
    ASSERT(!ended());
    push_back(ActionFactory::build(name, parametrisation_));
}


void ActionPlan::add(const std::string& name, const std::string& param, long value) {
    ASSERT(!ended());

    auto runtime = new param::RuntimeParametrisation(parametrisation_);
    runtimes_.push_back(runtime);
    runtime->set(param, value);
    push_back(ActionFactory::build(name, *runtime));
}


void ActionPlan::add(const std::string& name, const std::string& param, const std::string& value) {
    ASSERT(!ended());

    auto runtime = new param::RuntimeParametrisation(parametrisation_);
    runtimes_.push_back(runtime);
    runtime->set(param, value);
    push_back(ActionFactory::build(name, *runtime));
}


void ActionPlan::add(const std::string& name, const std::string& param1, const std::string& value1,
                     const std::string& param2, long value2) {
    ASSERT(!ended());

    auto runtime = new param::RuntimeParametrisation(parametrisation_);
    runtimes_.push_back(runtime);
    runtime->set(param1, value1);
    runtime->set(param2, value2);
    push_back(ActionFactory::build(name, *runtime));
}


void ActionPlan::add(const std::string& name, const std::string& param1, const std::string& value1,
                     const std::string& param2, const std::string& value2) {
    ASSERT(!ended());

    auto runtime = new param::RuntimeParametrisation(parametrisation_);
    runtimes_.push_back(runtime);
    runtime->set(param1, value1);
    runtime->set(param2, value2);
    push_back(ActionFactory::build(name, *runtime));
}


void ActionPlan::add(Action* action) {
    ASSERT(!ended());

    push_back(action);
}


void ActionPlan::add(const std::string& name, param::MIRParametrisation* runtime) {
    ASSERT(!ended());

    ASSERT(runtime);
    runtimes_.push_back(runtime);
    push_back(ActionFactory::build(name, *runtime, false));
}


void ActionPlan::execute(context::Context& ctx) const {
    ASSERT(ended());

    if (!dumpPlanFile_.empty()) {
        if (dumpPlanFile_ == "-") {
            custom(std::cout);
            std::cout << std::endl;
        }
        else {
            std::ofstream out(dumpPlanFile_, std::ios::app);
            custom(out);
            out << std::endl;
        }
    }

    bool dryrun = false;
    if (parametrisation_.get("dryrun", dryrun) && dryrun) {
        return;
    }

    const char* sep = "###################################################################################";

    for (const auto& p : *this) {
        Log::debug() << "Executing:"
                     << "\n"
                     << sep << "\n"
                     << *p << "\n"
                     << sep << std::endl;
        p->perform(ctx);
        Log::debug() << "Result:"
                     << "\n"
                     << sep << "\n"
                     << ctx << "\n"
                     << sep << std::endl;
    }

    if (!dumpStatisticsFile_.empty()) {
        if (dumpStatisticsFile_ == "-") {
            eckit::JSON out(std::cout);
            ctx.statistics().json(out);
        }
        else {
            std::ofstream file(dumpStatisticsFile_, std::ios::app);
            eckit::JSON out(file);
            ctx.statistics().json(out);
        }
    }
}


void ActionPlan::estimate(context::Context& ctx, api::MIREstimation& estimation) const {
    ASSERT(ended());

    for (const auto& p : *this) {
        Log::debug() << "Estimate " << (*p) << std::endl;
        p->estimate(ctx, estimation);
    }
}

void ActionPlan::compress() {
    ASSERT(ended());

    const char* sep = "#########";

    Log::debug() << "Compress:"
                 << "\n"
                 << sep << "\n"
                 << *this << "\n"
                 << sep << std::endl;

    bool hasCompressed = false;
    bool more          = true;
    while (more) {
        more = false;

        for (size_t i = 0; i < size() - 1; ++i) {
            std::ostringstream oldAction;
            oldAction << action(i);

            if (action(i).mergeWithNext(action(i + 1))) {

                Log::debug() << "ActionPlan::compress: "
                             << "\n   " << oldAction.str() << "\n + " << action(i + 1) << "\n = " << action(i)
                             << std::endl;

                delete at(i + 1);
                erase(begin() + long(i + 1));

                hasCompressed = true;
                more          = true;
                break;
            }

            if (action(i).deleteWithNext(action(i + 1))) {

                delete at(i);
                erase(begin() + long(i));

                hasCompressed = true;
                more          = true;
                break;
            }
        }
    }

    if (hasCompressed) {
        Log::debug() << "Compress result:"
                     << "\n"
                     << sep << "\n"
                     << *this << "\n"
                     << sep << std::endl;
    }
    else {
        Log::debug() << "Compress result: unable to compress"
                     << "\n"
                     << sep << std::endl;
    }
}


bool ActionPlan::ended() const {
    if (empty()) {
        return false;
    }
    return back()->isEndAction();
}


const Action& ActionPlan::action(size_t n) const {
    return *at(n);
}


Action& ActionPlan::action(size_t n) {
    return *at(n);
}


void ActionPlan::print(std::ostream& out) const {
    out << "ActionPlan[";
    const char* arrow = "";
    for (const auto& p : *this) {
        out << arrow << *p;
        arrow = " ==> ";
    }
    out << "]";
}


void ActionPlan::dump(std::ostream& out) const {
    for (const auto& p : *this) {
        out << "      ==> " << *p << std::endl;
    }
}


void ActionPlan::custom(std::ostream& out) const {
    const char* sep = "";
    for (const auto& p : *this) {
        out << sep;
        p->custom(out);
        sep = "|";
    }
}


}  // namespace action
}  // namespace mir
