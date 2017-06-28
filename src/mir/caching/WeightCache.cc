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


namespace mir {
namespace caching {


static std::string extract_loader(const param::MIRParametrisation& param) {
    std::string name;
    param.get("interpolator-loader", name);
    return name;
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
    return 4;
}


const char *WeightCacheTraits::extension() {
    return ".mat";
}


void WeightCacheTraits::save(const eckit::CacheManagerBase&, const value_type& W, const eckit::PathName& path) {
    eckit::Log::info() << "Inserting weights in cache : " << path << "" << std::endl;

    eckit::TraceTimer<LibMir> timer("Saving weights to cache");
    W.save(path);
}


void WeightCacheTraits::load(const eckit::CacheManagerBase& manager, value_type& W, const eckit::PathName& path) {

    eckit::TraceTimer<LibMir> timer("Loading weights from cache");

    using namespace mir::caching::interpolator;
    InterpolatorLoader* loader_ = InterpolatorLoaderFactory::build(manager.loader(), path);

    value_type w(loader_->address(), loader_->size());
    std::swap(W, w);

    W.validate("fromCache");
}


}  // namespace caching
}  // namespace mir

