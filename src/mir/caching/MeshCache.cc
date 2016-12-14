/*
 * (C) Copyright 1996-2014 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "mir/caching/MeshCache.h"

// #include "eckit/filesystem/PathName.h"
// #include "eckit/serialisation/FileStream.h"
// #include "eckit/config/Resource.h"

// #include "atlas/mesh/Mesh.h"

#include "mir/config/LibMir.h"
// #include "eckit/config/Resource.h"

using namespace eckit;

namespace mir {
namespace caching {


MeshCache::MeshCache():
    CacheManager<MeshCacheTraits>(LibMir::cacheDir(),
                                  eckit::Resource<bool>("$MIR_THROW_ON_CACHE_MISS;mirThrowOnCacheMiss",
                                                        false)) {
}

const char *MeshCacheTraits::name() {
    return "mir/meshes";
}

int MeshCacheTraits::version() {
    return 1;
}

const char *MeshCacheTraits::extension() {
    return ".mesh";
}

void MeshCacheTraits::save(const CacheManagerBase&, value_type&, const eckit::PathName&) {
    NOTIMP;
}

void MeshCacheTraits::load(const CacheManagerBase&, value_type&, const eckit::PathName&) {
    NOTIMP;
}
//----------------------------------------------------------------------------------------------------------------------

} // namespace caching
} // namespace mir

