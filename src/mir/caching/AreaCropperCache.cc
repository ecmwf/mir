// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/caching/AreaCropperCache.h"

#include "eckit/config/Resource.h"

#include "mir/config/LibMir.h"


namespace mir::caching {


AreaCropperCache::AreaCropperCache() :
    eckit::CacheManager<AreaCropperCacheTraits>(
        "Cropper",  // dummy -- would be used in load() / save() static functions
        LibMir::cacheDir(), eckit::Resource<bool>("$MIR_THROW_ON_CACHE_MISS;mirThrowOnCacheMiss", false),
        eckit::Resource<size_t>("$MIR_AREA_CACHE_SIZE", 0)) {}


const char* AreaCropperCacheTraits::name() {
    return "mir/area";
}


int AreaCropperCacheTraits::version() {
    return 3;
}


const char* AreaCropperCacheTraits::extension() {
    return ".crop";
}


void AreaCropperCacheTraits::save(const eckit::CacheManagerBase& /*unused*/, const value_type& c,
                                  const eckit::PathName& path) {
    c.save(path);
}


void AreaCropperCacheTraits::load(const eckit::CacheManagerBase& /*unused*/, value_type& c,
                                  const eckit::PathName& path) {
    c.load(path);
}


}  // namespace mir::caching
