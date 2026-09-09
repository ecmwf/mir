// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/plan/Executor.h"

#include <map>
#include <set>

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir::action {


static util::recursive_mutex* local_mutex  = nullptr;
static std::map<std::string, Executor*>* m = nullptr;
static util::once_flag once;
static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new std::map<std::string, Executor*>();
}


Executor::Executor(const std::string& name) : name_(name) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}

Executor::~Executor() {
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    ASSERT(m->find(name_) != m->end());
    m->erase(name_);
}


void Executor::list(std::ostream& out, bool full) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

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
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

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


}  // namespace mir::action
