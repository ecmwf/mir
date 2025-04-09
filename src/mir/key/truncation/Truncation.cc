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


#include "Truncation.h"

#include <algorithm>
#include <cctype>  // for ::isdigit
#include <map>
#include <ostream>

#include "mir/key/truncation/Ordinal.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir::key::truncation {


static util::once_flag once;
static util::recursive_mutex* local_mutex           = nullptr;
static std::map<std::string, TruncationFactory*>* m = nullptr;
static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new std::map<std::string, TruncationFactory*>();
}


Truncation::Truncation(const param::MIRParametrisation& parametrisation) : parametrisation_(parametrisation) {}


TruncationFactory::TruncationFactory(const std::string& name) : name_(name) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    if (m->find(name) != m->end()) {
        throw exception::SeriousBug("TruncationFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


TruncationFactory::~TruncationFactory() {
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    m->erase(name_);
}


Truncation* TruncationFactory::build(const std::string& name, const param::MIRParametrisation& parametrisation,
                                     long targetGaussianN) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    Log::debug() << "TruncationFactory: looking for '" << name << "'" << std::endl;
    ASSERT(!name.empty());

    if (auto j = m->find(name); j != m->end()) {
        return j->second->make(parametrisation, targetGaussianN);
    }

    // Look for a plain number
    if (!name.empty() && std::all_of(name.begin(), name.end(), ::isdigit)) {
        return new truncation::Ordinal(std::stol(name), parametrisation);
    }

    list(Log::error() << "TruncationFactory: unknown '" << name << "', choices are: ");
    throw exception::SeriousBug("TruncationFactory: unknown '" + name + "'");
}


void TruncationFactory::list(std::ostream& out) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }

    out << sep << "<ordinal>";
}


}  // namespace mir::key::truncation
