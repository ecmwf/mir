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


#include <iostream>

#include "eckit/config/Resource.h"
#include "eckit/log/Timer.h"

#include "mir/action/context/Context.h"
#include "mir/action/plan/ActionGraph.h"
#include "mir/action/plan/Executor.h"
#include "mir/action/plan/Job.h"
#include "mir/api/MIRComplexJob.h"
#include "mir/api/MIRJob.h"
#include "mir/api/MIRWatcher.h"
#include "mir/input/MIRInput.h"


namespace mir {
namespace api {


MIRComplexJob::MIRComplexJob() : input_(0) {}


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

    if (jobs_.empty()) {
        return;
    }

    static bool printActionGraph = eckit::Resource<bool>("$MIR_PRINT_ACTION_GRAPH", false);

    action::ActionGraph graph;

    size_t i = 0;
    for (auto j = jobs_.begin(); j != jobs_.end(); ++j, ++i) {
        graph.add((*j)->plan(), watchers_[i]);
    }

    if (!input_) {
        return;
    }

    std::unique_ptr<eckit::Timer> timer;

    if (printActionGraph) {
        timer.reset(new eckit::Timer("MIRComplexJob::execute", eckit::Log::info()));
    }

    context::Context ctx(*input_, statistics);

    if (printActionGraph) {
        eckit::Log::info() << ">>>>>>>>>>>> ====== " << std::endl;

        eckit::Log::info() << *input_ << std::endl;
    }

    const action::Executor& executor = action::Executor::lookup((*jobs_.begin())->parametrisation());

    if (printActionGraph) {
        graph.dump(eckit::Log::info(), 1);
    }
    graph.execute(ctx, executor);

    executor.wait();

    if (printActionGraph) {
        eckit::Log::info() << "<<<<<<<<<<< ======" << std::endl;
    }
}


bool MIRComplexJob::empty() const {
    return jobs_.empty();
}


void MIRComplexJob::print(std::ostream& out) const {
    out << "MIRComplexJob[]";
}


MIRComplexJob& MIRComplexJob::add(api::MIRJob* job, input::MIRInput& input, output::MIROutput& output,
                                  api::MIRWatcher* watcher) {

    if (!job) {
        return *this;
    }

    if (!input_) {
        input_ = &input;
    }

    if (input_ != &input) {
        std::ostringstream oss;
        oss << "MIRComplexJob: all jobs must share the same input (for now)";
        throw eckit::SeriousBug(oss.str());
    }


    apis_.push_back(job);  // We keep it becase the Job needs a reference
    jobs_.push_back(new action::Job(*job, input, output, false));
    watchers_.push_back(watcher);

    return *this;
}


}  // namespace api
}  // namespace mir
