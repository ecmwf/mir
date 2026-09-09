// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/caching/LegendreCache.h"

#include "eckit/config/Resource.h"

#include "mir/config/LibMir.h"


namespace mir::caching {


LegendreCache::LegendreCache() :
    eckit::CacheManager<LegendreCacheTraits>(
        "LegendreCache",  // dummy -- would be used in load() / save() static functions
        LibMir::cacheDir(), eckit::Resource<bool>("$MIR_THROW_ON_CACHE_MISS;mirThrowOnCacheMiss", false),
        eckit::Resource<size_t>("$MIR_COEFFS_CACHE_SIZE", 0)) {}

const char* LegendreCacheTraits::name() {
    return "mir/legendre";
}

int LegendreCacheTraits::version() {
    return 4;
}

const char* LegendreCacheTraits::extension() {
    return ".leg";
}

void LegendreCacheTraits::save(const eckit::CacheManagerBase& /*unused*/,
                               const LegendreCacheTraits::value_type& /*unused*/, const eckit::PathName& /*unused*/) {}

void LegendreCacheTraits::load(const eckit::CacheManagerBase& /*unused*/, LegendreCacheTraits::value_type& /*unused*/,
                               const eckit::PathName& /*unused*/) {}


}  // namespace mir::caching
