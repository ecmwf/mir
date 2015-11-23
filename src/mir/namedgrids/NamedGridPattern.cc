/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015

#include "mir/namedgrids/NamedGridPattern.h"

#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/exception/Exceptions.h"

#include <sstream>

namespace mir {
namespace namedgrids {
namespace {


static eckit::Mutex *local_mutex = 0;
static std::map<std::string, NamedGridPattern *> *m = 0;


static pthread_once_t once = PTHREAD_ONCE_INIT;

static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map<std::string, NamedGridPattern *>();
}


}  // (anonymous namespace)


NamedGridPattern::NamedGridPattern(const std::string &pattern):
    pattern_(pattern) {
    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    ASSERT(m->find(pattern) == m->end());
    (*m)[pattern] = this;
}


NamedGridPattern::~NamedGridPattern() {
    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    ASSERT(m->find(pattern_) != m->end());
    m->erase(pattern_);
}


void NamedGridPattern::list(std::ostream &out) {
    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char *sep = "";
    for (std::map<std::string, NamedGridPattern *>::const_iterator j = m->begin() ; j != m->end() ; ++j) {
        out << sep << (*j).first;
        sep = ", ";
    }
}


const NamedGrid *NamedGridPattern::build(const std::string &name) {

    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    std::map<std::string, NamedGridPattern *>::const_iterator j;

    eckit::Log::info() << "Looking for NamedGridPattern [" << name << "]" << std::endl;

    std::map<std::string, NamedGridPattern *>::const_iterator k = m->end();
    for (j = m->begin() ; j != m->end() ; ++j) {
        if (j->second->pattern_.match(name)) {
            eckit::Log::info() << "Regex " << j->second->pattern_ << " and " << name << " (match)" << std::endl;

            if (k != m->end()) {
                std::stringstream os;
                os << "gridname " << name << " matches " << *(k->second) << " and " << *(j->second);
                throw eckit::SeriousBug(os.str());
            }
            k = j;
        } else {
            eckit::Log::info() << "Regex " << j->second->pattern_ << " and " << name << " (no match)" << std::endl;
        }
    }

    if (k != m->end()) {
        return k->second->make(name);
    }


    if (k == m->end()) {
        eckit::Log::error() << "No NamedGridPattern for [" << name << "]" << std::endl;
        eckit::Log::error() << "NamedGridPattern are:" << std::endl;
        for (j = m->begin() ; j != m->end() ; ++j)
            eckit::Log::error() << "   " << *(*j).second << std::endl;
    }

    return 0;
}


}  // namespace logic
}  // namespace mir

