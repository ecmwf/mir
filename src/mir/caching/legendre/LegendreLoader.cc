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

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"

#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace caching {
namespace legendre {


LegendreLoader::LegendreLoader(const param::MIRParametrisation& parametrisation, const eckit::PathName& path) :
    parametrisation_(parametrisation),
    path_(path.realName()) {}


LegendreLoader::~LegendreLoader() = default;


eckit::Channel& LegendreLoader::log() {
    static auto& channel = eckit::Log::debug<LibMir>();
    return channel;
}


eckit::Channel& LegendreLoader::info() {
    static auto& channel = eckit::Log::info();
    return channel;
}


eckit::Channel& LegendreLoader::warn() {
    static auto& channel = eckit::Log::warning();
    return channel;
}


static pthread_once_t once                              = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex                        = nullptr;
static std::map<std::string, LegendreLoaderFactory*>* m = nullptr;
static void init() {
    local_mutex = new eckit::Mutex();
    m           = new std::map<std::string, LegendreLoaderFactory*>();
}


LegendreLoaderFactory::LegendreLoaderFactory(const std::string& name) : name_(name) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    if (m->find(name) != m->end()) {
        throw eckit::SeriousBug("LegendreLoaderFactory: duplicate '" + name + "'");
    }

    ASSERT(m->find(name) == m->end());
    (*m)[name] = this;
}


LegendreLoaderFactory::~LegendreLoaderFactory() {
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    m->erase(name_);
}


LegendreLoader* LegendreLoaderFactory::build(const param::MIRParametrisation& params, const eckit::PathName& path) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    std::string name = "mapped-memory";
    params.get("legendre-loader", name);

    eckit::Log::debug<LibMir>() << "LegendreLoaderFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(eckit::Log::error() << "LegendreLoaderFactory: unknown '" << name << "', choices are: ");
        throw eckit::SeriousBug("LegendreLoaderFactory: unknown '" + name + "'");
    }

    return j->second->make(params, path);
}


bool LegendreLoaderFactory::inSharedMemory(const param::MIRParametrisation& params) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    std::string name = "mapped-memory";
    params.get("legendre-loader", name);

    eckit::Log::debug<LibMir>() << "LegendreLoaderFactory: looking for '" << name << "'" << std::endl;

    auto j = m->find(name);
    if (j == m->end()) {
        list(eckit::Log::error() << "LegendreLoaderFactory: unknown '" << name << "', choices are: ");
        throw eckit::SeriousBug("LegendreLoaderFactory: unknown '" + name + "'");
    }

    return j->second->shared();
}


void LegendreLoaderFactory::list(std::ostream& out) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    const char* sep = "";
    for (const auto& j : *m) {
        out << sep << j.first;
        sep = ", ";
    }
}


}  // namespace legendre
}  // namespace caching
}  // namespace mir
