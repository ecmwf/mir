/*
 * (C) Copyright 1996- ECMWF.
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

#include "mir/namedgrids/NamedGrid.h"
#include "mir/namedgrids/NamedGridPattern.h"

#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/exception/Exceptions.h"
#include "mir/config/LibMir.h"


namespace mir {
namespace namedgrids {
namespace {


static eckit::Mutex *local_mutex = 0;
static std::map<std::string, NamedGrid *> *m = 0;


static pthread_once_t once = PTHREAD_ONCE_INIT;

static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map<std::string, NamedGrid *>();
}


}  // (anonymous namespace)


NamedGrid::NamedGrid(const std::string &name):
    name_(name) {
    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


NamedGrid::~NamedGrid() {
    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    ASSERT(m->find(name_) != m->end());
    m->erase(name_);
}


void NamedGrid::list(std::ostream& out) {
    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (std::map<std::string, NamedGrid *>::const_iterator j = m->begin() ; j != m->end() ; ++j) {
        out << sep << (*j).first;
        sep = ", ";
    }
}


const NamedGrid& NamedGrid::lookup(const std::string &name) {

    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    std::map<std::string, NamedGrid *>::const_iterator j = m->find(name);

    eckit::Log::debug<LibMir>() << "Looking for NamedGrid [" << name << "]" << std::endl;

    if (j == m->end()) {

        // Look for pattern matchings

        // This will automatically add the new NamedGrid to the map
        const NamedGrid *ng = NamedGridPattern::build(name);
        if(ng) {
            return *ng;
        }

        eckit::Log::error() << "No NamedGrid for [" << name << "]" << std::endl;
        eckit::Log::error() << "NamedGrid are:" << std::endl;
        for (j = m->begin() ; j != m->end() ; ++j)
            eckit::Log::error() << "   " << (*j).first << std::endl;
        throw eckit::SeriousBug(std::string("No NamedGrid called ") + name);
    }

    return *(*j).second;
}


}  // namespace namedgrids
}  // namespace mir

