/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "LegendreCache.h"

#include "mir/api/mir_version.h"
#include "mir/config/LibMir.h"
#include "eckit/config/Resource.h"

#include "eckit/filesystem/PathName.h"

using namespace eckit;

namespace mir {
namespace caching {

//----------------------------------------------------------------------------------------------------------------------

LegendreCache::LegendreCache() :
    CacheManager("mir/coeffs", LibMir::cacheDir(), eckit::Resource<bool>("$MIR_THROW_ON_CACHE_MISS;mirThrowOnCacheMiss", false)) {}

const char* LegendreCache::version() const {
    return "1"; // Change me if the cache file structure changes
}
const char* LegendreCache::extension() const {
    return ".leg";
}


void LegendreCache::print(std::ostream &s) const {
    s << "LegendreCache[";
    CacheManager::print(s);
    s << "name=" << name() << ","
      << "version=" << version() << ","
      << "extention=" << extension() << ","
      << "]";
}

//----------------------------------------------------------------------------------------------------------------------

}  // namespace method
}  // namespace mir

