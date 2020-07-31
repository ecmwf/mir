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

#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"

#include "mir/config/LibMir.h"


namespace mir {
namespace key {
namespace grid {


static eckit::Mutex* local_mutex              = nullptr;
static std::map<std::string, GridPattern*>* m = nullptr;
static pthread_once_t once                    = PTHREAD_ONCE_INIT;
static void init() {
    local_mutex = new eckit::Mutex();
    m           = new std::map<std::string, GridPattern*>();
}


GridPattern::GridPattern(const std::string& pattern) : pattern_(pattern) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    ASSERT(m->find(pattern) == m->end());
    (*m)[pattern] = this;
}


GridPattern::~GridPattern() {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    ASSERT(m->find(pattern_) != m->end());
    m->erase(pattern_);
}


void GridPattern::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    auto sep = "";
    for (auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


bool GridPattern::match(const std::string& name) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    eckit::Log::debug<LibMir>() << "GridPattern: looking for '" << name << "'" << std::endl;

    bool conflicts = false;
    auto k         = m->cend();
    for (auto j = m->cbegin(); j != m->cend() && !conflicts; ++j) {
        if (j->second->pattern_.match(name)) {
            conflicts = k != m->end();
            k         = j;
        }
    }

    bool can = !conflicts && k != m->end();
    eckit::Log::debug<LibMir>() << "GridPattern: '" << name << "' " << (can ? "can" : "cannot") << " be built"
                                << std::endl;
    return can;
}


const Grid* GridPattern::build(const std::string& name) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    eckit::Log::debug<LibMir>() << "GridPattern: looking for '" << name << "'" << std::endl;

    auto k = m->end();
    for (auto j = m->begin(); j != m->end(); ++j) {
        if (j->second->pattern_.match(name)) {
            eckit::Log::debug<LibMir>() << "GridPattern: '" << j->second->pattern_ << "' match" << std::endl;

            if (k != m->end()) {
                std::stringstream os;
                os << "GridPattern: '" << name << "' matches '" << k->second << "' and '" << j->second << "'"
                   << std::endl;
                throw eckit::SeriousBug(os.str());
            }
            k = j;
        }
        else {
            eckit::Log::debug<LibMir>() << "GridPattern: '" << j->second->pattern_ << "' no match" << std::endl;
        }
    }

    if (k != m->end()) {
        return k->second->make(name);
    }


    if (k == m->end()) {
        list(eckit::Log::error() << "GridPattern: unknown '" << name << "', choices are: ");
        eckit::Log::error() << std::endl;
    }

    return nullptr;
}


}  // namespace grid
}  // namespace key
}  // namespace mir
