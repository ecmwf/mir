/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


#include "mir/style/Resol.h"

#include <algorithm>
#include <cctype>  // for ::isdigit
#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"
#include "mir/config/LibMir.h"
#include "mir/namedgrids/NamedGridPattern.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/style/resol/NamedGrid.h"
#include "mir/style/resol/Truncation.h"


namespace mir {
namespace style {


void Resol::prepare(action::ActionPlan&) const {
    std::ostringstream os;
    os << "Resol::prepare() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


//=========================================================================


namespace {
static pthread_once_t once = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex = 0;
static std::map< std::string, ResolFactory* >* m = 0;
static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map< std::string, ResolFactory* >();
}
}  // (anonymous namespace)


ResolFactory::ResolFactory(const std::string& name) : name_(name) {
    pthread_once(&once, init);

    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("ResolFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


ResolFactory::~ResolFactory() {
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    m->erase(name_);
}


Resol* ResolFactory::build(const std::string& name, const param::MIRParametrisation& parametrisation) {
    pthread_once(&once, init);
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    eckit::Log::debug<LibMir>() << "ResolFactory: looking for '" << name << "'" << std::endl;
    ASSERT(!name.empty());

    auto j = m->find(name);
    if (j != m->end()) {
        return (*j).second->make(parametrisation);
    }

    // Look for NamedGrid pattern matching
    if (namedgrids::NamedGridPattern::match(name)) {
        return new resol::NamedGrid(name, parametrisation);
    }

    // Look for "T" + number, or just a plain number
    const std::string possibleNumber = (name.length() > 1 && name[0] == 'T') ? name.substr(1) : name;
    if (std::all_of(possibleNumber.begin(), possibleNumber.end(), ::isdigit)) {
        long number = std::stol(possibleNumber);
        return new resol::Truncation(number, parametrisation);
    }

    list(eckit::Log::error() << "ResolFactory: unknown '" << name << "', choices are: ");
    throw eckit::SeriousBug("ResolFactory: unknown '" + name + "'");
}


void ResolFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }

    namedgrids::NamedGridPattern::list(out << sep);
    sep = ", ";

    out << sep << "T<ordinal>"
        << sep << "<ordinal>";
}


}  // namespace style
}  // namespace mir
