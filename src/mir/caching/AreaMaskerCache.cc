// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/caching/AreaMaskerCache.h"

#include "eckit/config/Resource.h"

#include "mir/config/LibMir.h"


namespace mir::caching {


AreaMaskerCache::AreaMaskerCache() :
    eckit::CacheManager<AreaMaskerCacheTraits>(
        "Masker",  // dummy -- would be used in load() / save() static functions
        LibMir::cacheDir(), eckit::Resource<bool>("$MIR_THROW_ON_CACHE_MISS;mirThrowOnCacheMiss", false),
        eckit::Resource<size_t>("$MIR_AREA_CACHE_SIZE", 0)) {}


const char* AreaMaskerCacheTraits::name() {
    return "mir/area";
}


int AreaMaskerCacheTraits::version() {
    return 3;
}


const char* AreaMaskerCacheTraits::extension() {
    return ".mask";
}


void AreaMaskerCacheTraits::save(const eckit::CacheManagerBase& /*unused*/, const value_type& c,
                                 const eckit::PathName& path) {
    c.save(path);
}


void AreaMaskerCacheTraits::load(const eckit::CacheManagerBase& /*unused*/, value_type& c,
                                 const eckit::PathName& path) {
    c.load(path);
}


}  // namespace mir::caching
