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


#include "mir/caching/LegendreCache.h"

#include "mir/config/LibMir.h"


namespace mir {
namespace caching {


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


}  // namespace caching
}  // namespace mir
