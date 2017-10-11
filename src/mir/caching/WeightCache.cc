/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "mir/caching/WeightCache.h"

#include "eckit/io/Buffer.h"
#include "mir/caching/interpolator/InterpolatorLoader.h"
#include "mir/config/LibMir.h"
#include "mir/method/WeightMatrix.h"
#include "eckit/os/AutoUmask.h"

#include <unistd.h>

namespace mir {
namespace caching {

using namespace mir::caching::interpolator;

//----------------------------------------------------------------------------------------------------------------------


static std::string extract_loader(const param::MIRParametrisation& param) {

    std::string name;
    if (param.get("interpolator-loader", name)) {
        return name;
    }

    return "file-io";
}


WeightCache::WeightCache(const param::MIRParametrisation& param):
    CacheManager(extract_loader(param),
                 LibMir::cacheDir(),
                 eckit::Resource<bool>("$MIR_THROW_ON_CACHE_MISS;mirThrowOnCacheMiss", false)) {
}


const char *WeightCacheTraits::name() {
    return "mir/weights";
}


int WeightCacheTraits::version() {
    return 7;
}


const char *WeightCacheTraits::extension() {
    return ".mat";
}


void WeightCacheTraits::save(const eckit::CacheManagerBase&, const value_type& W, const eckit::PathName& path) {
    eckit::Log::debug<LibMir>() << "Inserting weights in cache : " << path << "" << std::endl;
    eckit::TraceTimer<LibMir> timer("Saving weights to cache");
    W.save(path);
}

void WeightCacheTraits::load(const eckit::CacheManagerBase& manager, value_type& w, const eckit::PathName& path) {

    eckit::TraceTimer<LibMir> timer("Loading weights from cache");

    value_type tmp( InterpolatorLoaderFactory::build(manager.loader(), path) );
    w.swap(tmp);

    w.validate("fromCache");
}

//----------------------------------------------------------------------------------------------------------------------

// We only lock per host, not per cluster

static eckit::PathName lockFile(const std::string& path) {
    eckit::AutoUmask umask(0);

    eckit::PathName lock(path + ".lock");
    lock.touch();
    return lock;
}

WeightCacheLock::WeightCacheLock(const std::string& path):
    path_(lockFile(path)),
    lock_(path_) {
}

void WeightCacheLock::lock() {
    eckit::AutoUmask umask(0);

    eckit::Log::info() << "Wait for lock " << path_ << std::endl;
    lock_.lock();
    eckit::Log::info() << "Got lock " << path_ << std::endl;


    char hostname[1024];
    SYSCALL(gethostname(hostname, sizeof(hostname) - 1));

    std::ofstream os(path_.asString().c_str());
    os << hostname << " " << ::getpid() << std::endl;

}

void WeightCacheLock::unlock() {
    eckit::AutoUmask umask(0);

    eckit::Log::info() << "Unlock " << path_ << std::endl;
    std::ofstream os(path_.asString().c_str());
    os << std::endl;
    lock_.unlock();
}

//----------------------------------------------------------------------------------------------------------------------


}  // namespace caching
}  // namespace mir

