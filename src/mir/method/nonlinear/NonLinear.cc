// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/nonlinear/NonLinear.h"

#include <map>

#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir::method::nonlinear {


static util::recursive_mutex* local_mutex          = nullptr;
static std::map<std::string, NonLinearFactory*>* m = nullptr;
static util::once_flag once;
static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new std::map<std::string, NonLinearFactory*>();
}


NonLinear::NonLinear(const param::MIRParametrisation& /*unused*/) {}


NonLinear::~NonLinear() = default;


NonLinearFactory::NonLinearFactory(const std::string& name) : name_(name) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    if (m->find(name) == m->end()) {
        (*m)[name] = this;
        return;
    }
    throw exception::SeriousBug("NonLinearFactory: duplicated NonLinear '" + name + "'");
}


NonLinearFactory::~NonLinearFactory() {
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    m->erase(name_);
}


const NonLinear* NonLinearFactory::build(const std::string& name, const param::MIRParametrisation& param) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    Log::debug() << "NonLinearFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(Log::error() << "NonLinearFactory: unknown '" << name << "', choices are:\n");
        throw exception::SeriousBug("NonLinearFactory: unknown '" + name + "'");
    }

    return j->second->make(param);
}


void NonLinearFactory::list(std::ostream& out) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    const char* sep = "";
    for (auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace mir::method::nonlinear
