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


#include "mir/key/style/MIRStyle.h"

#include <map>

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir {
namespace key {
namespace style {


MIRStyle::MIRStyle(const param::MIRParametrisation& parametrisation) : parametrisation_(parametrisation) {}


MIRStyle::~MIRStyle() = default;


static util::once_flag once;
static util::recursive_mutex* local_mutex         = nullptr;
static std::map<std::string, MIRStyleFactory*>* m = nullptr;
static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new std::map<std::string, MIRStyleFactory*>();
}


MIRStyleFactory::MIRStyleFactory(const std::string& name) : name_(name) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    if (m->find(name) != m->end()) {
        throw exception::SeriousBug("MIRStyleFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


MIRStyleFactory::~MIRStyleFactory() {
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    m->erase(name_);
}


MIRStyle* MIRStyleFactory::build(const param::MIRParametrisation& params) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    std::string name;
    if (!params.get("style", name)) {
        throw exception::SeriousBug("MIRStyleFactory: cannot get 'style'");
    }

    Log::debug() << "MIRStyleFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(Log::error() << "MIRStyleFactory: unknown '" << name << "', choices are: ");
        throw exception::SeriousBug("MIRStyleFactory: unknown '" + name + "'");
    }

    return j->second->make(params);
}


void MIRStyleFactory::list(std::ostream& out) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace style
}  // namespace key
}  // namespace mir
