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


#include "mir/key/grid/GridPattern.h"

#include <map>
#include <sstream>

#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir::key::grid {


static util::recursive_mutex* local_mutex     = nullptr;
static std::map<std::string, GridPattern*>* m = nullptr;
static util::once_flag once;
static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new std::map<std::string, GridPattern*>();
}


GridPattern::GridPattern(const std::string& pattern) : pattern_(pattern), regex_(pattern_) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    ASSERT(m->find(pattern) == m->end());
    (*m)[pattern] = this;
}


GridPattern::~GridPattern() {
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    ASSERT(m->find(pattern_) != m->end());
    m->erase(pattern_);
}


void GridPattern::list(std::ostream& out) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    const auto* sep = "";
    for (auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}

std::string GridPattern::match(const std::string& name, const param::MIRParametrisation& param) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    Log::debug() << "GridPattern: looking for '" << name << "'" << std::endl;

    bool conflicts = false;
    auto k         = m->cend();
    for (auto j = m->cbegin(); j != m->cend() && !conflicts; ++j) {
        if (j->second->regex_.match(name)) {
            conflicts = k != m->cend();
            k         = j;
        }
    }

    if (!conflicts && k != m->cend()) {
        Log::debug() << "GridPattern: '" << name << "' can be built" << std::endl;
        return k->second->canonical(name, param);
    }

    Log::debug() << "GridPattern: '" << name << "' cannot be built" << std::endl;
    return "";
}


const Grid* GridPattern::lookup(const std::string& name) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    Log::debug() << "GridPattern: looking for '" << name << "'" << std::endl;

    auto k = m->cend();
    for (auto j = m->cbegin(); j != m->cend(); ++j) {
        if (j->second->regex_.match(name)) {
            Log::debug() << "GridPattern: '" << j->second->pattern_ << "' match" << std::endl;

            if (k != m->cend()) {
                std::ostringstream os;
                os << "GridPattern: '" << name << "' matches '" << k->second << "' and '" << j->second << "'"
                   << std::endl;
                throw exception::SeriousBug(os.str());
            }
            k = j;
        }
        else {
            Log::debug() << "GridPattern: '" << j->second->pattern_ << "' no match" << std::endl;
        }
    }

    if (k != m->cend()) {
        // This adds a new Grid to the map
        return k->second->make(name);
    }


    list(Log::error() << "GridPattern: unknown '" << name << "', choices are: ");
    throw exception::SeriousBug("GridPattern: unknown '" + name + "'");
}


}  // namespace mir::key::grid
