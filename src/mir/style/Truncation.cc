/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


#include "Truncation.h"

#include <algorithm>
#include <cctype>  // for ::isdigit
#include <iostream>
#include <map>

#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"

#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/style/truncation/Ordinal.h"


namespace mir {
namespace style {


namespace {
static pthread_once_t once = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex = nullptr;
static std::map< std::string, TruncationFactory* >* m = nullptr;
static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map< std::string, TruncationFactory* >();
}
}  // (anonymous namespace)


Truncation::Truncation(const param::MIRParametrisation& parametrisation) :
    parametrisation_(parametrisation) {
}


TruncationFactory::TruncationFactory(const std::string& name) : name_(name) {
    pthread_once(&once, init);

    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("TruncationFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


TruncationFactory::~TruncationFactory() {
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    m->erase(name_);
}


Truncation* TruncationFactory::build(
        const std::string& name,
        const param::MIRParametrisation& parametrisation,
        long targetGaussianN ) {
    pthread_once(&once, init);
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    eckit::Log::debug<LibMir>() << "TruncationFactory: looking for '" << name << "'" << std::endl;
    ASSERT(!name.empty());

    auto j = m->find(name);
    if (j != m->end()) {
        return (*j).second->make(parametrisation, targetGaussianN);
    }

    // Look for a plain number
    if (std::all_of(name.begin(), name.end(), ::isdigit)) {
        long number = std::stol(name);
        return new truncation::Ordinal(number, parametrisation);
    }

    list(eckit::Log::error() << "TruncationFactory: unknown '" << name << "', choices are: ");
    throw eckit::SeriousBug("TruncationFactory: unknown '" + name + "'");
}


void TruncationFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock< eckit::Mutex > lock(local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }

    out << sep << "<ordinal>";
}


}  // namespace style
}  // namespace mir
