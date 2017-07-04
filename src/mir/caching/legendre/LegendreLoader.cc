/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @author Tiago Quintino
///
/// @date Apr 2015

#include "eckit/exception/Exceptions.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/thread/Once.h"

#include "mir/caching/legendre/LegendreLoader.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"

namespace mir {
namespace caching {
namespace legendre {

//----------------------------------------------------------------------------------------------------------------------

static eckit::Mutex* local_mutex = 0;
static std::map<std::string, LegendreLoaderFactory*>* m = 0;

static pthread_once_t once = PTHREAD_ONCE_INIT;

static void init() {
    local_mutex = new eckit::Mutex();
    m = new std::map<std::string, LegendreLoaderFactory*>();
}

//----------------------------------------------------------------------------------------------------------------------

LegendreLoader::LegendreLoader(const param::MIRParametrisation& parametrisation, const eckit::PathName& path)
    : parametrisation_(parametrisation), path_(path.realName()) {}

LegendreLoader::~LegendreLoader() {}

//----------------------------------------------------------------------------------------------------------------------

LegendreLoaderFactory::LegendreLoaderFactory(const std::string& name) : name_(name) {
    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}

LegendreLoaderFactory::~LegendreLoaderFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    m->erase(name_);
}

LegendreLoader* LegendreLoaderFactory::build(const param::MIRParametrisation& params, const eckit::PathName& path) {
    pthread_once(&once, init);

    std::string name = "mapped-memory";
    params.get("legendre-loader", name);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    std::map<std::string, LegendreLoaderFactory*>::const_iterator j = m->find(name);

    eckit::Log::debug<LibMir>() << "Looking for LegendreLoaderFactory [" << name << "]" << std::endl;

    if (j == m->end()) {
        eckit::Log::error() << "No LegendreLoaderFactory for [" << name << "]" << std::endl;
        eckit::Log::error() << "LegendreLoaderFactories are:" << std::endl;
        for (j = m->begin(); j != m->end(); ++j) eckit::Log::error() << "   " << (*j).first << std::endl;
        throw eckit::SeriousBug(std::string("No LegendreLoaderFactory called ") + name);
    }

    return (*j).second->make(params, path);
}

void LegendreLoaderFactory::list(std::ostream& out) {
    pthread_once(&once, init);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (std::map<std::string, LegendreLoaderFactory*>::const_iterator j = m->begin(); j != m->end(); ++j) {
        out << sep << (*j).first;
        sep = ", ";
    }
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace legendre
}  // namespace caching
}  // namespace mir
