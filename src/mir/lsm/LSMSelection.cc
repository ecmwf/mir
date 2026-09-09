// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/lsm/LSMSelection.h"

#include <map>

#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir::lsm {


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

    if (auto j = m->find(name); j != m->end()) {
        return *(j->second);
    }

    list(Log::error() << "LSMSelection: unknown '" << name << "', choices are: ");
    throw exception::SeriousBug("LSMSelection: unknown '" + name + "'");
}


}  // namespace mir::lsm
