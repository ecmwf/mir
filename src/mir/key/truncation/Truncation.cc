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
#include <mutex>
#include <ostream>

#include "mir/key/truncation/Ordinal.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir {
namespace key {
namespace truncation {


static std::once_flag once;
static std::mutex* local_mutex                      = nullptr;
static std::map<std::string, TruncationFactory*>* m = nullptr;
static void init() {
    local_mutex = new std::mutex();
    m           = new std::map<std::string, TruncationFactory*>();
}


Truncation::Truncation(const param::MIRParametrisation& parametrisation) : parametrisation_(parametrisation) {}


TruncationFactory::TruncationFactory(const std::string& name) : name_(name) {
    std::call_once(once, init);
    std::lock_guard<std::mutex> lock(*local_mutex);

    if (m->find(name) != m->end()) {
        throw exception::SeriousBug("TruncationFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


TruncationFactory::~TruncationFactory() {
    std::lock_guard<std::mutex> lock(*local_mutex);

    m->erase(name_);
}


Truncation* TruncationFactory::build(const std::string& name, const param::MIRParametrisation& parametrisation,
                                     long targetGaussianN) {
    {
        std::call_once(once, init);
        std::lock_guard<std::mutex> lock(*local_mutex);

        Log::debug() << "TruncationFactory: looking for '" << name << "'" << std::endl;
        ASSERT(!name.empty());

        auto j = m->find(name);
        if (j != m->end()) {
            return j->second->make(parametrisation, targetGaussianN);
        }

        // Look for a plain number
        if (std::all_of(name.begin(), name.end(), ::isdigit)) {
            long number = std::stol(name);
            return new truncation::Ordinal(number, parametrisation);
        }
    }

    list(Log::error() << "TruncationFactory: unknown '" << name << "', choices are: ");
    throw exception::SeriousBug("TruncationFactory: unknown '" + name + "'");
}


void TruncationFactory::list(std::ostream& out) {
    std::call_once(once, init);
    std::lock_guard<std::mutex> lock(*local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }

    out << sep << "<ordinal>";
}


}  // namespace truncation
}  // namespace key
}  // namespace mir
