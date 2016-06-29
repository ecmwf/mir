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

#include "mir/action/plan/Executor.h"
#include "mir/action/plan/Action.h"
#include "mir/param/RuntimeParametrisation.h"
#include "mir/action/context/Context.h"
#include "mir/log/MIR.h"
#include "mir/api/MIRWatcher.h"
#include "mir/param/MIRParametrisation.h"

#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/exception/Exceptions.h"



namespace mir {
namespace action {

namespace {


static eckit::Mutex *local_mutex = 0;
static std::map<std::string, Executor *> *m = 0;


static pthread_once_t once = PTHREAD_ONCE_INIT;

static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map<std::string, Executor *>();
}

}

Executor::Executor(const std::string &name):
    name_(name) {
    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}

Executor::~Executor()  {
    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    ASSERT(m->find(name_) != m->end());
    m->erase(name_);

}

void Executor::list(std::ostream & out) {
    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);


    std::set<std::string> seen;
    const char *sep = "";
    for (std::map<std::string, Executor *>::const_iterator j = m->begin() ; j != m->end() ; ++j) {
        std::string name = (*j).first.substr(0, (*j).first.find("."));
        if (seen.find(name) == seen.end()) {
            out << sep << name;
            sep = ", ";
            seen.insert(name);
        }
    }
}

const Executor &Executor::lookup(const param::MIRParametrisation& params) {

    pthread_once(&once, init);

    std::string name;

    if (!params.get("executor", name)) {
        throw eckit::SeriousBug("Executor cannot get executor");
    }

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    std::map<std::string, Executor *>::const_iterator j = m->find(name);

    eckit::Log::trace<MIR>() << "Looking for Executor [" << name << "]" << std::endl;
    if (j == m->end()) {
        eckit::Log::error() << "No Executor for [" << name << "]" << std::endl;
        eckit::Log::error() << "Executors are:" << std::endl;
        for (j = m->begin() ; j != m->end() ; ++j)
            eckit::Log::error() << "   " << (*j).first << std::endl;
        throw eckit::SeriousBug(std::string("No Executor called ") + name);
    }


    return *(*j).second;
}


}  // namespace action
}  // namespace mir

