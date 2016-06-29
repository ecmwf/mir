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

#include "mir/action/plan/ThreadExecutor.h"
#include "mir/action/plan/Action.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/action/context/Context.h"
#include "mir/log/MIR.h"
#include "mir/api/MIRWatcher.h"
#include "mir/action/plan/ActionNode.h"

#include "eckit/exception/Exceptions.h"

#include "eckit/thread/ThreadPool.h"


namespace mir {
namespace action {

static eckit::ThreadPool* pool = 0;

static pthread_once_t once = PTHREAD_ONCE_INIT;

static void init() {
    pool = new eckit::ThreadPool("executor", 2);
}


class ThreadExecutorTask : public eckit::ThreadPoolTask {
    const ThreadExecutor& owner_;
    context::Context ctx_; // Not a reference, so we have a copy
    const ActionNode& node_;

    virtual void execute() {
        std::cout << "===> Execute " << node_ << std::endl;
        node_.execute(ctx_, owner_);
        std::cout << "<=== Done " << node_ << std::endl;

    }

public:
    ThreadExecutorTask(const ThreadExecutor& owner,
                       context::Context& ctx,
                       const ActionNode& node):
        owner_(owner),
        ctx_(ctx),
        node_(node) {

    }
};

ThreadExecutor::ThreadExecutor(const std::string& name):
    Executor(name) {

}

ThreadExecutor::~ThreadExecutor() {

}

void ThreadExecutor::print(std::ostream& out) const {
    out << "ThreadExecutor[]";
}

void ThreadExecutor::wait() const {
    pthread_once(&once, init);
    pool->wait();
}

void ThreadExecutor::execute(context::Context& ctx, const ActionNode& node) const {
    pthread_once(&once, init);
    pool->push(new ThreadExecutorTask(*this, ctx, node));
}


void ThreadExecutor::parametrisation(const param::MIRParametrisation &parametrisation) {
    pthread_once(&once, init);
    size_t threads;
    if (parametrisation.get("executor.threads", threads)) {
        pool->resize(threads);
    }
}


namespace {
static ThreadExecutor executor("thread");
}

}  // namespace action
}  // namespace mir

