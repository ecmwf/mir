/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/data/Space.h"

#include <map>

#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"
#include "mir/config/LibMir.h"


namespace mir {
namespace data {


Space::Space() {
}


Space::~Space() = default;


//=========================================================================


namespace {
static pthread_once_t once = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex = 0;
static std::map< std::string, SpaceChooser* >* m = 0;
static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map< std::string, SpaceChooser* >();
}
}  // (anonymous namespace)


SpaceChooser::SpaceChooser(const std::string& name, Space* choice, size_t component, size_t dimensions) :
    name_(name),
    choice_(choice),
    component_(component),
    dimensions_(dimensions) {
    pthread_once(&once, init);
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("SpaceChooser: duplicate '" + name + "'");
    }

    if (component_ >= dimensions_) {
        throw eckit::SeriousBug("SpaceChooser: '" + name + "' component (" + std::to_string(component_) + ") is not below dimensions (" + std::to_string(dimensions_) + ")");
    }

    (*m)[name] = this;
}


SpaceChooser::~SpaceChooser() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    delete choice_;
    m->erase(name_);
}


const Space& SpaceChooser::lookup(const std::string& name) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    eckit::Log::debug<LibMir>() << "SpaceChooser: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(eckit::Log::error() << "SpaceChooser: unknown '" << name << "', choices are: ");
        throw eckit::SeriousBug("SpaceChooser: unknown '" + name + "'");
    }

    return *((j->second)->choice_);
}


void SpaceChooser::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace data
}  // namespace mir

