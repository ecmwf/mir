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
#include "mir/config/LibMir.h"
#include "mir/method/WeightMatrix.h"
#include "mir/caching/interpolator/InterpolatorLoader.h"


namespace mir {
namespace caching {

//----------------------------------------------------------------------------------------------------------------------

WeightCache::WeightCache(const param::MIRParametrisation& parametrisation):
    CacheManager(LibMir::cacheDir(),
                 eckit::Resource<bool>("$MIR_THROW_ON_CACHE_MISS;mirThrowOnCacheMiss", false)),
    parametrisation_(parametrisation) {
}

const char *WeightCacheTraits::name() {
    return "mir/weights";
}

int WeightCacheTraits::version() {
    return 2;
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

    const WeightCache& wcache = static_cast<const WeightCache&>(manager);

    using namespace mir::caching::interpolator;

    InterpolatorLoader* loader_ = InterpolatorLoaderFactory::build(wcache.parametrisation_, path);

    value_type w;
//    value_type w(loader_->address(), loader_->size());

    std::swap(W, w);

    W.validate("fromCache");
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace method
}  // namespace mir

