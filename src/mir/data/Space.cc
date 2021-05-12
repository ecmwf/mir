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


#include "mir/data/Space.h"

#include <map>

#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir {
namespace data {


Space::Space() = default;


Space::~Space() = default;


static util::once_flag once;
static util::recursive_mutex* local_mutex      = nullptr;
static std::map<std::string, SpaceChooser*>* m = nullptr;
static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new std::map<std::string, SpaceChooser*>();
}


SpaceChooser::SpaceChooser(const std::string& name, Space* choice, size_t component, size_t dimensions) :
    name_(name), choice_(choice), component_(component), dimensions_(dimensions) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    if (m->find(name) != m->end()) {
        throw exception::SeriousBug("SpaceChooser: duplicate '" + name + "'");
    }

    if (component_ >= dimensions_) {
        throw exception::SeriousBug("SpaceChooser: '" + name + "' component (" + std::to_string(component_) +
                                    ") is not below dimensions (" + std::to_string(dimensions_) + ")");
    }

    (*m)[name] = this;
}


SpaceChooser::~SpaceChooser() {
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    delete choice_;
    m->erase(name_);
}


const Space& SpaceChooser::lookup(const std::string& name) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    Log::debug() << "SpaceChooser: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(Log::error() << "SpaceChooser: unknown '" << name << "', choices are: ");
        throw exception::SeriousBug("SpaceChooser: unknown '" + name + "'");
    }

    return *((j->second)->choice_);
}


void SpaceChooser::list(std::ostream& out) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace data
}  // namespace mir
