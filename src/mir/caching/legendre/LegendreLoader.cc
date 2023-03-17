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


#include "mir/caching/legendre/LegendreLoader.h"

#include <map>

#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir::caching::legendre {


LegendreLoader::LegendreLoader(const param::MIRParametrisation& parametrisation, const eckit::PathName& path) :
    parametrisation_(parametrisation), path_(path.realName()) {}


LegendreLoader::~LegendreLoader() = default;


atlas::trans::LegendreCache LegendreLoader::transCache() {
    return {address(), size()};
}


static util::once_flag once;
static util::recursive_mutex* local_mutex               = nullptr;
static std::map<std::string, LegendreLoaderFactory*>* m = nullptr;
static void init() {
    local_mutex = new util::recursive_mutex();
    m           = new std::map<std::string, LegendreLoaderFactory*>();
}


LegendreLoaderFactory::LegendreLoaderFactory(const std::string& name) : name_(name) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    if (m->find(name) != m->end()) {
        throw exception::SeriousBug("LegendreLoaderFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


LegendreLoaderFactory::~LegendreLoaderFactory() {
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    m->erase(name_);
}


LegendreLoader* LegendreLoaderFactory::build(const param::MIRParametrisation& params, const eckit::PathName& path) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> guard(*local_mutex);

    std::string name = LibMir::cacheLoader(LibMir::cache_loader::LEGENDRE);
    params.get("legendre-loader", name);

    Log::debug() << "LegendreLoaderFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(Log::error() << "LegendreLoaderFactory: unknown '" << name << "', choices are: ");
        throw exception::SeriousBug("LegendreLoaderFactory: unknown '" + name + "'");
    }

    return j->second->make(params, path);
}


bool LegendreLoaderFactory::inSharedMemory(const param::MIRParametrisation& params) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> guard(*local_mutex);

    std::string name = LibMir::cacheLoader(LibMir::cache_loader::LEGENDRE);
    params.get("legendre-loader", name);

    Log::debug() << "LegendreLoaderFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(Log::error() << "LegendreLoaderFactory: unknown '" << name << "', choices are: ");
        throw exception::SeriousBug("LegendreLoaderFactory: unknown '" + name + "'");
    }

    return j->second->shared();
}


void LegendreLoaderFactory::list(std::ostream& out) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> guard(*local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace mir::caching::legendre
