// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <ostream>
#include <sstream>

#include "eckit/config/Resource.h"
#include "eckit/log/JSON.h"
#include "eckit/spec/Spec.h"

#include "mir/action/context/Context.h"
#include "mir/action/plan/ActionGraph.h"
#include "mir/action/plan/Dump.h"
#include "mir/action/plan/Executor.h"
#include "mir/action/plan/Job.h"
#include "mir/api/MIRComplexJob.h"
#include "mir/api/MIRJob.h"
#include "mir/api/MIRWatcher.h"
#include "mir/data/MIRField.h"
#include "mir/input/MIRInput.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Trace.h"


namespace mir::api {


MIRComplexJob::MIRComplexJob() : input_(nullptr) {}


MIRComplexJob::~MIRComplexJob() {
    clear();
}


void MIRComplexJob::clear() {
    for (auto& j : jobs_) {
        delete j;
    }
    jobs_.clear();

    for (auto& j : apis_) {
        delete j;
    }
    apis_.clear();

    for (auto& j : watchers_) {
        delete j;
    }
    watchers_.clear();

    input_ = nullptr;
}


void MIRComplexJob::execute(util::MIRStatistics& statistics) const {
    static bool printActionGraph = eckit::Resource<bool>("$MIR_PRINT_ACTION_GRAPH", false);

    if (jobs_.empty() || input_ == nullptr) {
        return;
    }

    action::ActionGraph graph;
    size_t i = 0;
    for (const auto& j : jobs_) {
        graph.add(j->plan(), watchers_[i++]);
    }

    // dump options, as the first job's (as the executor)
    const auto& param = jobs_.front()->parametrisation().userParametrisation();

    std::string dumpActionPlanFile;
    if (param.get("dump-action-plan-file", dumpActionPlanFile)) {
        action::append_line(dumpActionPlanFile, [&graph](std::ostream& out) {
            eckit::JSON j(out);
            graph.json(j);
        });
    }

    std::string dumpGridPlanFile;
    if (param.get("dump-grid-plan-file", dumpGridPlanFile)) {
        graph.recordGrids();
    }

    std::unique_ptr<trace::Timer> timer(printActionGraph ? new trace::Timer("MIRComplexJob::execute") : nullptr);

    if (printActionGraph) {
        Log::info() << ">>>>>>>>>>>>"
                       "\n"
                    << *input_ << std::endl;
        graph.dump(Log::info(), 1);
    }

    context::Context ctx(*input_, statistics);

    std::unique_ptr<repres::RepresentationHandle> input;
    if (!dumpGridPlanFile.empty()) {
        input = std::make_unique<repres::RepresentationHandle>(ctx.field().representation());
    }

    const auto& executor = action::Executor::lookup(jobs_.front()->parametrisation());
    graph.execute(ctx, executor);
    executor.wait();

    if (input) {
        action::append_line(dumpGridPlanFile, [&input, &graph](std::ostream& out) {
            eckit::JSON j(out);
            j.startList();
            (*input)->spec().json(j);
            graph.json_grids(j);
            j.endList();
        });
    }

    if (printActionGraph) {
        Log::info() << "<<<<<<<<<<<<" << std::endl;
    }
}


bool MIRComplexJob::empty() const {
    return jobs_.empty();
}


void MIRComplexJob::print(std::ostream& out) const {
    out << "MIRComplexJob[]";
}


MIRComplexJob& MIRComplexJob::add(MIRJob* job, input::MIRInput& input, output::MIROutput& output, MIRWatcher* watcher) {

    if (job == nullptr) {
        return *this;
    }

    if (input_ == nullptr) {
        input_ = &input;
    }

    if (input_ != &input) {
        std::ostringstream oss;
        oss << "MIRComplexJob: all jobs must share the same input (for now)";
        throw exception::SeriousBug(oss.str());
    }


    apis_.push_back(job);  // We keep it becase the Job needs a reference
    jobs_.push_back(new action::Job(*job, input, output, false));
    watchers_.push_back(watcher);

    return *this;
}


}  // namespace mir::api
