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
    return 1;
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
