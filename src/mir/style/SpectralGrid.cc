/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @date May 2017


#include "mir/style/SpectralGrid.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"
#include "mir/config/LibMir.h"
#include "mir/style/SpectralNamedGrid.h"


namespace mir {
namespace style {


SpectralGrid::SpectralGrid(const param::MIRParametrisation& parametrisation) :
    parametrisation_(parametrisation) {
}


SpectralGrid::~SpectralGrid() {
}


void SpectralGrid::get(const std::string& name, std::string& value) const {
    ASSERT(name == "gridname");
    value = getGridname();
}


//=========================================================================


namespace {
static pthread_once_t once = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex = 0;
static std::map< std::string, SpectralGridFactory* > *m = 0;
static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map< std::string, SpectralGridFactory* >();
}
}  // (anonymous namespace)


SpectralGridFactory::SpectralGridFactory(const std::string& name) : name_(name) {
    pthread_once(&once, init);
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("SpectralGridFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


SpectralGridFactory::~SpectralGridFactory() {
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    m->erase(name_);
}


SpectralGrid* SpectralGridFactory::build(const std::string& name, const param::MIRParametrisation& parametrisation) {
    pthread_once(&once, init);
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    eckit::Log::debug<LibMir>() << "SpectralGridFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j != m->end()) {
        return j->second->make(parametrisation);
    }

    eckit::Log::info() << "SpectralGridFactory: setting named grid '" << name << "'" << std::endl;
    return new SpectralNamedGrid(parametrisation);
}


void SpectralGridFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    const char* sep = "";
    for (auto j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace style
}  // namespace mir

