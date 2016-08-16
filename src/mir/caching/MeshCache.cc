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

#include "eckit/filesystem/PathName.h"
#include "eckit/serialisation/FileStream.h"
#include "eckit/config/Resource.h"

#include "atlas/mesh/Mesh.h"

#include "mir/api/LibMir.h"

using namespace eckit;

namespace mir {
namespace caching {


MeshCache::MeshCache():
    CacheManager("mir/meshes", LibMir::cacheDir()) {
}

const char *MeshCache::version() const {
    return "1"; // Change me if the cache file structure changes
}

const char *MeshCache::extension() const {
    return ".gmsh";
}

void MeshCache::print(std::ostream &s) const {
    s << "MeshCache[";
    CacheManager::print(s);
    s << "name=" << name() << ","
      << "version=" << version() << ","
      << "extention=" << extension() << ","
      << "]";
}

void MeshCache::insert(const std::string &key, const atlas::mesh::Mesh &mesh) const {

    eckit::PathName tmp = stage(key);


    eckit::FileStream s(tmp, "w");
    mesh.encode(s);


    ASSERT(commit(key, tmp));
}

bool MeshCache::retrieve(const std::string &key, atlas::mesh::Mesh &mesh) const {

    eckit::PathName path;

    if (!get(key, path))
        return false;

    // eckit::Log::info() << "Found cropping in cache : " << path << "" << std::endl;
    // eckit::TraceTimer<MIR> timer("Loading cropping from cache");


    eckit::FileStream s(path, "r");
    NOTIMP;
    //mesh = atlas::mesh::Mesh(s);

    return true;
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace caching
} // namespace mir

