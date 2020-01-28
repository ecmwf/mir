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


#include "mir/style/MIRStyle.h"

#include <map>

#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"

#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace style {
MIRStyle::MIRStyle(const param::MIRParametrisation& parametrisation) : parametrisation_(parametrisation) {}


MIRStyle::~MIRStyle() = default;


//=========================================================================


namespace {
static pthread_once_t once                        = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex                  = nullptr;
static std::map<std::string, MIRStyleFactory*>* m = nullptr;
static void init() {
    local_mutex = new eckit::Mutex();
    m           = new std::map<std::string, MIRStyleFactory*>();
}
}  // namespace


MIRStyleFactory::MIRStyleFactory(const std::string& name) : name_(name) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("MIRStyleFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


MIRStyleFactory::~MIRStyleFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    m->erase(name_);
}


MIRStyle* MIRStyleFactory::build(const param::MIRParametrisation& params) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    std::string name;
    if (!params.get("style", name)) {
        throw eckit::SeriousBug("MIRStyleFactory: cannot get 'style'");
    }

    eckit::Log::debug<LibMir>() << "MIRStyleFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(eckit::Log::error() << "MIRStyleFactory: unknown '" << name << "', choices are: ");
        throw eckit::SeriousBug("MIRStyleFactory: unknown '" + name + "'");
    }

    return (*j).second->make(params);
}


void MIRStyleFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace style
}  // namespace mir
