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


#include "mir/method/nonlinear/NonLinear.h"

#include <map>
#include <mutex>

#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir {
namespace method {
namespace nonlinear {


static std::recursive_mutex* local_mutex           = nullptr;
static std::map<std::string, NonLinearFactory*>* m = nullptr;
static std::once_flag once;
static void init() {
    local_mutex = new std::recursive_mutex();
    m           = new std::map<std::string, NonLinearFactory*>();
}


NonLinear::NonLinear(const param::MIRParametrisation&) {}


bool NonLinear::modifiesMatrix() const {
    return false;
}


NonLinear::~NonLinear() = default;


NonLinearFactory::NonLinearFactory(const std::string& name) : name_(name) {
    std::call_once(once, init);
    std::lock_guard<std::recursive_mutex> lock(*local_mutex);

    if (m->find(name) == m->end()) {
        (*m)[name] = this;
        return;
    }
    throw exception::SeriousBug("NonLinearFactory: duplicated NonLinear '" + name + "'");
}


NonLinearFactory::~NonLinearFactory() {
    std::lock_guard<std::recursive_mutex> lock(*local_mutex);

    m->erase(name_);
}


const NonLinear* NonLinearFactory::build(const std::string& name, const param::MIRParametrisation& param) {
    std::call_once(once, init);
    std::lock_guard<std::recursive_mutex> lock(*local_mutex);

    Log::debug() << "NonLinearFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(Log::error() << "NonLinearFactory: unknown '" << name << "', choices are:\n");
        throw exception::SeriousBug("NonLinearFactory: unknown '" + name + "'");
    }

    return j->second->make(param);
}


void NonLinearFactory::list(std::ostream& out) {
    std::call_once(once, init);
    std::lock_guard<std::recursive_mutex> lock(*local_mutex);

    const char* sep = "";
    for (auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace nonlinear
}  // namespace method
}  // namespace mir
