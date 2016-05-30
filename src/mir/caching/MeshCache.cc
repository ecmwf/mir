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

using namespace eckit;

namespace mir {
namespace caching {

using atlas::mesh::Mesh;

//----------------------------------------------------------------------------------------------------------------------

PathName MeshCache::filename(const std::string& key)
{
    PathName base_path = Resource<PathName>("$MIR_CACHE_DIR", "~mir-cache/tmp/cache/mir" );

    PathName f = base_path / "mesh" / PathName( key + ".gmsh" );

    return f;
}

bool MeshCache::add(const std::string& key, const Mesh& mesh)
{
    PathName file( filename(key) );

    if( file.exists() )
    {
        Log::info() << "MeshCache entry " << file << " already exists ..." << std::endl;
        return false;
    }

    file.dirName().mkdir();  // ensure directory exists

    // unique file name avoids race conditions on the file from multiple processes

    PathName tmpfile ( PathName::unique(file) );

    Log::info() << "Inserting mesh in cache (" << file << ")" << std::endl;

    {
        eckit::FileStream s(tmpfile,"w");
        mesh.encode(s);
    }


    // now try to rename the file to its file pathname
    try
    {
        PathName::rename(tmpfile, file);
    }
    catch( FailedSystemCall& e ) // ignore failed system call -- another process may have created the file meanwhile
    {
        Log::info() << "Failed rename of cache file -- " << e.what() << std::endl;
    }

    return true;
}

atlas::mesh::Mesh* MeshCache::get(const std::string &key)
{
    PathName file( filename(key) );

    Log::info() << "Looking for file cache (" << file << ")" << std::endl;

    if(!file.exists()) { return 0; }

    Log::info() << "Found mesh in cache (" << file << ")" << std::endl;

    eckit::FileStream s(file, "r");

    return new Mesh(s);
}

//----------------------------------------------------------------------------------------------------------------------

} // namespace caching
} // namespace mir

