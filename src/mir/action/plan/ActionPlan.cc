// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/plan/ActionPlan.h"

#include <ostream>
#include <sstream>

#include "eckit/log/JSON.h"
#include "eckit/spec/Spec.h"

#include "mir/action/context/Context.h"
#include "mir/action/plan/Action.h"
#include "mir/action/plan/Dump.h"
#include "mir/data/MIRField.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"


namespace mir::action {


namespace {


std::string get(const param::MIRParametrisation& param, const std::string& key) {
    std::string value;
    param.get(key, value);
    return value;
}


/// Grids (specs), as a JSON list
template <typename Grids>
void json_grids(eckit::JSON& j, const Grids& grids) {
    j.startList();
    for (const auto& grid : grids) {
        (*grid)->spec().json(j);
    }
    j.endList();
}


}  // namespace


ActionPlan::ActionPlan(const param::MIRParametrisation& param) :
    parametrisation_(param),
    dumpPlanFile_(get(param.userParametrisation(), "dump-plan-file")),
    dumpStatisticsFile_(get(param.userParametrisation(), "dump-statistics-file")),
    dumpActionPlanFile_(get(param.userParametrisation(), "dump-action-plan-file")),
    dumpGridPlanFile_(get(param.userParametrisation(), "dump-grid-plan-file")) {}


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

    auto* runtime = new param::RuntimeParametrisation(parametrisation_);
    runtimes_.push_back(runtime);
    runtime->set(param, value);
    push_back(ActionFactory::build(name, *runtime));
}


void ActionPlan::add(const std::string& name, const std::string& param, const std::string& value) {
    ASSERT(!ended());

    auto* runtime = new param::RuntimeParametrisation(parametrisation_);
    runtimes_.push_back(runtime);
    runtime->set(param, value);
    push_back(ActionFactory::build(name, *runtime));
}


void ActionPlan::add(const std::string& name, const std::string& param1, const std::string& value1,
                     const std::string& param2, long value2) {
    ASSERT(!ended());

    auto* runtime = new param::RuntimeParametrisation(parametrisation_);
    runtimes_.push_back(runtime);
    runtime->set(param1, value1);
    runtime->set(param2, value2);
    push_back(ActionFactory::build(name, *runtime));
}


void ActionPlan::add(const std::string& name, const std::string& param1, const std::string& value1,
                     const std::string& param2, const std::string& value2) {
    ASSERT(!ended());

    auto* runtime = new param::RuntimeParametrisation(parametrisation_);
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
        append_line(dumpPlanFile_, [this](std::ostream& out) { custom(out); });
    }

    if (!dumpActionPlanFile_.empty()) {
        append_line(dumpActionPlanFile_, [this](std::ostream& out) { out << json_str(); });
    }

    if (bool dryrun = false; parametrisation_.userParametrisation().get("dryrun", dryrun) && dryrun) {
        return;
    }

    const auto grids = run(ctx, !dumpGridPlanFile_.empty());

    if (!dumpGridPlanFile_.empty()) {
        append_line(dumpGridPlanFile_, [&grids](std::ostream& out) {
            eckit::JSON j(out);
            json_grids(j, grids);
        });
    }

    if (!dumpStatisticsFile_.empty()) {
        append_line(dumpStatisticsFile_, [&ctx](std::ostream& out) {
            eckit::JSON j(out);
            ctx.statistics().json(j);
        });
    }
}


std::string ActionPlan::executeGridPlan(context::Context& ctx) const {
    ASSERT(ended());

    std::ostringstream out;
    eckit::JSON j(out);
    json_grids(j, run(ctx, true));
    return out.str();
}


ActionPlan::grids_t ActionPlan::run(context::Context& ctx, bool recordGrids) const {
    grids_t grids;
    auto record = [&grids, &ctx, recordGrids]() {
        if (recordGrids) {
            grids.emplace_back(std::make_unique<repres::RepresentationHandle>(ctx.field().representation()));
        }
    };

    const char* sep = "###################################################################################";

    record();
    for (const auto& p : *this) {
        if (Log::debug_active()) {
            Log::debug() << "Executing:"
                         << "\n"
                         << sep << "\n"
                         << *p << "\n"
                         << sep << std::endl;
        }

        p->perform(ctx);
        record();

        if (Log::debug_active()) {
            Log::debug() << "Result:"
                         << "\n"
                         << sep << "\n"
                         << ctx << "\n"
                         << sep << std::endl;
        }
    }

    return grids;
}


void ActionPlan::json(eckit::JSON& j) const {
    j.startList();
    for (const auto& p : *this) {
        p->json(j, true);
    }
    j.endList();
}


std::string ActionPlan::json_str() const {
    std::ostringstream out;
    eckit::JSON j(out);
    json(j);
    return out.str();
}


void ActionPlan::compress() {
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


}  // namespace mir::action
