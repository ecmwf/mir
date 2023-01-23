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


#include "mir/action/plan/ThreadExecutor.h"

#include "eckit/thread/ThreadPool.h"

#include "mir/action/context/Context.h"
#include "mir/action/plan/ActionNode.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir {
namespace action {


static util::once_flag once;
static eckit::ThreadPool* pool = nullptr;
static void init() {
    pool = new eckit::ThreadPool("executor", 2);
}


class ThreadExecutorTask : public eckit::ThreadPoolTask {
    const ThreadExecutor& owner_;
    context::Context ctx_;  // Not a reference, so we have a copy
    const ActionNode& node_;

    void execute() override {
        Log::info() << "===> Execute " << node_ << std::endl;
        node_.execute(ctx_, owner_);
        Log::info() << "<=== Done " << node_ << std::endl;
    }

public:
    ThreadExecutorTask(const ThreadExecutor& owner, context::Context& ctx, const ActionNode& node) :
        owner_(owner), ctx_(ctx), node_(node) {}
    ~ThreadExecutorTask() override = default;

    ThreadExecutorTask(const ThreadExecutorTask&)            = delete;
    ThreadExecutorTask(ThreadExecutorTask&&)                 = delete;
    ThreadExecutorTask& operator=(const ThreadExecutorTask&) = delete;
    ThreadExecutorTask& operator=(ThreadExecutorTask&&)      = delete;
};


ThreadExecutor::ThreadExecutor(const std::string& name) : Executor(name) {}


void ThreadExecutor::print(std::ostream& out) const {
    out << "ThreadExecutor[]";
}


void ThreadExecutor::wait() const {
    util::call_once(once, init);
    pool->wait();
}


void ThreadExecutor::execute(context::Context& ctx, const ActionNode& node) const {
    util::call_once(once, init);
    pool->push(new ThreadExecutorTask(*this, ctx, node));
}


void ThreadExecutor::parametrisation(const param::MIRParametrisation& parametrisation) {
    util::call_once(once, init);
    size_t threads;
    if (parametrisation.get("executor.threads", threads)) {
        pool->resize(threads);
    }
}


static const ThreadExecutor executor("thread");


}  // namespace action
}  // namespace mir
