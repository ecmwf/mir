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


#include "mir/style/IntermediateGrid.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"
#include "mir/config/LibMir.h"
#include "mir/namedgrids/NamedGrid.h"
#include "mir/style/IntermediateNamedGrid.h"


namespace mir {
namespace style {


IntermediateGrid::IntermediateGrid(const param::MIRParametrisation& parametrisation) :
    parametrisation_(parametrisation) {
}


IntermediateGrid::~IntermediateGrid() {
}


void IntermediateGrid::get(const std::string& name, std::string& value) const {
    ASSERT(name == "gridname");
    value = getGridname();
}


//=========================================================================


namespace {
static pthread_once_t once = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex = 0;
static std::map< std::string, IntermediateGridFactory* > *m = 0;
static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map< std::string, IntermediateGridFactory* >();
}
}  // (anonymous namespace)


IntermediateGridFactory::IntermediateGridFactory(const std::string& name) : name_(name) {
    pthread_once(&once, init);
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("IntermediateGridFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


IntermediateGridFactory::~IntermediateGridFactory() {
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    m->erase(name_);
}


IntermediateGrid* IntermediateGridFactory::build(const std::string& name, const param::MIRParametrisation& parametrisation) {
    pthread_once(&once, init);
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    eckit::Log::debug<LibMir>() << "IntermediateGridFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j != m->end()) {
        return j->second->make(parametrisation);
    }

    // look for a supported named grid
    try {
        namedgrids::NamedGrid::lookup(name);
    } catch (const eckit::SeriousBug&) {
        eckit::Log::error() << "IntermediateGridFactory: unknown '" << name << "'" << std::endl;
        throw;
    }

    eckit::Log::info() << "IntermediateGridFactory: setting grid name '" << name << "'" << std::endl;
    return new IntermediateNamedGrid(parametrisation);
}


void IntermediateGridFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace style
}  // namespace mir

