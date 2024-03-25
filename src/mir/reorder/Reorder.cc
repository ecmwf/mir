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


#include "mir/reorder/Reorder.h"

#include <map>
#include <ostream>

#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir::reorder {


static util::once_flag ONCE;
static util::recursive_mutex* MUTEX = nullptr;

static std::map<std::string, ReorderFactory*>* M = nullptr;


static void init() {
    MUTEX = new util::recursive_mutex();
    M     = new std::map<std::string, ReorderFactory*>();
}


ReorderFactory::ReorderFactory(const std::string& name) : name_(name) {
    util::call_once(ONCE, init);
    util::lock_guard<util::recursive_mutex> lock(*MUTEX);

    if (M->find(name) == M->end()) {
        (*M)[name] = this;
        return;
    }

    throw exception::SeriousBug("ReorderFactory: duplicated Reorder '" + name + "'");
}


ReorderFactory::~ReorderFactory() {
    util::lock_guard<util::recursive_mutex> lock(*MUTEX);
    M->erase(name_);
}


Reorder* ReorderFactory::build(const std::string& name) {
    util::call_once(ONCE, init);
    util::lock_guard<util::recursive_mutex> lock(*MUTEX);

    if (auto j = M->find(name); j != M->end()) {
        return j->second->make();
    }

    list(Log::error() << "ReorderFactory: unknown '" << name << "', choices are:\n") << std::endl;
    throw exception::SeriousBug("ReorderFactory: unknown '" + name + "'");
}


std::ostream& ReorderFactory::list(std::ostream& out) {
    util::call_once(ONCE, init);
    util::lock_guard<util::recursive_mutex> lock(*MUTEX);

    const auto* sep = "";
    for (const auto& j : *M) {
        out << sep << j.first;
        sep = ", ";
    }

    return out;
}


}  // namespace mir::reorder
