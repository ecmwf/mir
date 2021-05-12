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


#include "mir/lsm/LSMSelection.h"

#include <map>

#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir {
namespace lsm {


static util::once_flag once;
static util::recursive_mutex* local_mutex      = nullptr;
static std::map<std::string, LSMSelection*>* m = nullptr;
static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new std::map<std::string, LSMSelection*>();
}


LSMSelection::LSMSelection(const std::string& name) : name_(name) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


LSMSelection::~LSMSelection() {
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    ASSERT(m->find(name_) != m->end());
    m->erase(name_);
}


void LSMSelection::list(std::ostream& out) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


const LSMSelection& LSMSelection::lookup(const std::string& name) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    Log::debug() << "LSMSelection: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(Log::error() << "LSMSelection: unknown '" << name << "', choices are: ");
        throw exception::SeriousBug("LSMSelection: unknown '" + name + "'");
    }

    return *(j->second);
}


}  // namespace lsm
}  // namespace mir
