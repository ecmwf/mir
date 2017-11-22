/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


#include "mir/style/Intgrid.h"

#include <algorithm>
#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"
#include "mir/config/LibMir.h"
#include "mir/namedgrids/NamedGridPattern.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/style/intgrid/NamedGrid.h"


namespace mir {
namespace style {


namespace {
static pthread_once_t once = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex = 0;
static std::map< std::string, IntgridFactory* >* m = 0;
static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map< std::string, IntgridFactory* >();
}
}  // (anonymous namespace)


IntgridFactory::IntgridFactory(const std::string& name) : name_(name) {
    pthread_once(&once, init);

    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("IntgridFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


IntgridFactory::~IntgridFactory() {
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    m->erase(name_);
}


Intgrid* IntgridFactory::build(const std::string& name, const param::MIRParametrisation& parametrisation) {
    pthread_once(&once, init);
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    eckit::Log::debug<LibMir>() << "IntgridFactory: looking for '" << name << "'" << std::endl;
    ASSERT(!name.empty());

    auto j = m->find(name);
    if (j != m->end()) {
        return (*j).second->make(parametrisation);
    }

    // Look for NamedGrid pattern matching
    if (namedgrids::NamedGridPattern::match(name)) {
        return new intgrid::NamedGrid(name, parametrisation);
    }

    list(eckit::Log::error() << "IntgridFactory: unknown '" << name << "', choices are: ");
    throw eckit::SeriousBug("IntgridFactory: unknown '" + name + "'");
}


void IntgridFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }

    namedgrids::NamedGridPattern::list(out << sep);
}


}  // namespace style
}  // namespace mir
