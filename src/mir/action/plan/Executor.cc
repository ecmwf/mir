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
#include <mutex>
#include <set>

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir {
namespace action {


static std::mutex* local_mutex             = nullptr;
static std::map<std::string, Executor*>* m = nullptr;
static std::once_flag once;
static void init() {
    local_mutex = new std::mutex();
    m           = new std::map<std::string, Executor*>();
}


Executor::Executor(const std::string& name) : name_(name) {
    std::call_once(once, init);
    std::lock_guard<std::mutex> lock(*local_mutex);

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}

Executor::~Executor() {
    std::lock_guard<std::mutex> lock(*local_mutex);

    ASSERT(m->find(name_) != m->end());
    m->erase(name_);
}

void Executor::list(std::ostream& out, bool full) {
    std::call_once(once, init);
    std::lock_guard<std::mutex> lock(*local_mutex);

    std::set<std::string> seen;
    const char* sep = "";
    for (auto& j : *m) {
        std::string name = full ? j.first : j.first.substr(0, j.first.find("."));
        if (seen.find(name) == seen.end()) {
            out << sep << name;
            sep = ", ";
            seen.insert(name);
        }
    }
}

const Executor& Executor::lookup(const param::MIRParametrisation& params) {
    std::call_once(once, init);
    std::lock_guard<std::mutex> lock(*local_mutex);

    std::string name;
    if (!params.get("executor", name)) {
        throw exception::SeriousBug("Executor cannot get executor");
    }

    Log::debug() << "Looking for Executor [" << name << "]" << std::endl;

    auto j = m->find(name);
    if (j != m->end()) {
        j->second->parametrisation(params);
        return *(j->second);
    }

    list(Log::error() << "Executor: unknown '" << name << "', choices are: ", true);
    throw exception::SeriousBug("Executor: unknown '" + name + "'");
}


}  // namespace action
}  // namespace mir
