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


#include "mir/action/plan/Executor.h"

#include <map>
#include <set>

#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"

#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace action {


namespace {

static eckit::Mutex* local_mutex           = nullptr;
static std::map<std::string, Executor*>* m = nullptr;

static pthread_once_t once = PTHREAD_ONCE_INIT;

static void init() {
    local_mutex = new eckit::Mutex();
    m           = new std::map<std::string, Executor*>();
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

void Executor::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    std::set<std::string> seen;
    const char *sep = "";
    for (auto& j : *m) {
        std::string name = j.first.substr(0, j.first.find("."));
        if (seen.find(name) == seen.end()) {
            out << sep << name;
            sep = ", ";
            seen.insert(name);
        }
    }
}

const Executor& Executor::lookup(const param::MIRParametrisation& params) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    std::string name;
    if (!params.get("executor", name)) {
        throw eckit::SeriousBug("Executor cannot get executor");
    }

    eckit::Log::debug<LibMir>() << "Looking for Executor [" << name << "]" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        eckit::Log::error() << "No Executor for [" << name << "]" << std::endl;
        eckit::Log::error() << "Executors are:" << std::endl;
        for (auto& k : *m) {
            eckit::Log::error() << "   " << k.first << std::endl;
        }
        throw eckit::SeriousBug("No Executor called " + name);
    }


    (*j).second->parametrisation(params);

    return *(*j).second;
}


}  // namespace action
}  // namespace mir

