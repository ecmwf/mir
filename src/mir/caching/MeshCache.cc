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


#include "mir/caching/MeshCache.h"

#include "eckit/config/Resource.h"

#include "mir/config/LibMir.h"


namespace mir::caching {


MeshCache::MeshCache() :
    eckit::CacheManager<MeshCacheTraits>("Mesh",  // dummy -- would be used in load() / save() static functions
                                         LibMir::cacheDir(),
                                         eckit::Resource<bool>("$MIR_THROW_ON_CACHE_MISS;mirThrowOnCacheMiss", false),
                                         eckit::Resource<size_t>("$MIR_MESH_CACHE_SIZE", 0)) {}


const char* MeshCacheTraits::name() {
    return "mir/meshes";
}


int MeshCacheTraits::version() {
    return 2;
}


const char* MeshCacheTraits::extension() {
    return ".mesh";
}


void MeshCacheTraits::save(const eckit::CacheManagerBase& /*unused*/, const value_type& /*unused*/,
                           const eckit::PathName& /*unused*/) {
    NOTIMP;
}


void MeshCacheTraits::load(const eckit::CacheManagerBase& /*unused*/, value_type& /*unused*/,
                           const eckit::PathName& /*unused*/) {
    NOTIMP;
}


}  // namespace mir::caching
