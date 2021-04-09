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


#include "mir/caching/InMemoryMeshCache.h"

#include <mutex>
#include <ostream>

#include "eckit/utils/MD5.h"

#include "mir/method/fe/BuildNodeLumpedMassMatrix.h"
#include "mir/method/fe/CalculateCellLongestDiagonal.h"
#include "mir/util/Log.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/MeshGeneratorParameters.h"
#include "mir/util/Trace.h"
#include "mir/util/Types.h"


namespace mir {
namespace caching {


static std::mutex local_mutex;

constexpr size_t CAPACITY = 512 * 1024 * 1024;
static InMemoryCache<atlas::Mesh> mesh_cache("mirMesh", CAPACITY, 0, "$MIR_MESH_CACHE_MEMORY_FOOTPRINT");


const InMemoryMeshCache& InMemoryMeshCache::instance() {
    static InMemoryMeshCache cache;
    return cache;
}


atlas::Mesh InMemoryMeshCache::atlasMesh(util::MIRStatistics& statistics, const atlas::Grid& grid,
                                         const util::MeshGeneratorParameters& meshGeneratorParams) {
    std::lock_guard<std::mutex> guard(local_mutex);

    auto& log = Log::debug();
    trace::ResourceUsage usage_mesh("Mesh for grid " + grid.name() + " (" + grid.uid() + ")", log);
    auto cacheUse(statistics.cacheUser(mesh_cache));

    // generate signature including the mesh generation settings
    eckit::MD5 md5;
    md5 << grid;
    md5 << meshGeneratorParams;

    auto sign(md5.digest());
    auto j = mesh_cache.find(sign);
    if (j != mesh_cache.end()) {
        return *j;
    }

    atlas::Mesh& mesh = mesh_cache[sign];
    ASSERT(!mesh.generated());

    try {
        log << "InMemoryMeshCache: generating mesh using " << meshGeneratorParams << std::endl;

        atlas::MeshGenerator generator(meshGeneratorParams.meshGenerator_, meshGeneratorParams);
        mesh = generator.generate(grid);
        ASSERT(mesh.generated());

        // If meshgenerator did not create xyz field already, do it now.
        {
            trace::ResourceUsage timer("Mesh: BuildXYZField");
            atlas::mesh::actions::BuildXYZField()(mesh);
        }

        // Calculate barycenters of mesh cells
        if (meshGeneratorParams.meshCellCentres_) {
            trace::ResourceUsage timer("Mesh: BuildCellCentres");
            atlas::mesh::actions::BuildCellCentres()(mesh);
        }

        // Calculate the mesh cells longest diagonal
        if (meshGeneratorParams.meshCellLongestDiagonal_) {
            trace::ResourceUsage usage("CalculateCellLongestDiagonal");
            method::fe::CalculateCellLongestDiagonal()(mesh, grid.domain().global());
        }

        // Calculate node-lumped mass matrix
        if (meshGeneratorParams.meshNodeLumpedMassMatrix_) {
            trace::ResourceUsage timer("Mesh: BuildNodeLumpedMassMatrix");
            method::fe::BuildNodeLumpedMassMatrix()(mesh);
        }

        // Calculate node-to-cell ("inverse") connectivity
        if (meshGeneratorParams.meshNodeToCellConnectivity_) {
            trace::ResourceUsage timer("Mesh: BuildNode2CellConnectivity");
            atlas::mesh::actions::BuildNode2CellConnectivity{mesh}();
        }

        // Some information
        log << "Mesh[cells=" << Log::Pretty(mesh.cells().size()) << ",nodes=" << Log::Pretty(mesh.nodes().size()) << ","
            << meshGeneratorParams << "]" << std::endl;

        // Write file(s)
        if (!meshGeneratorParams.fileLonLat_.empty()) {
            atlas::output::PathName path(meshGeneratorParams.fileLonLat_);
            log << "Mesh: writing lonlat to '" << path << "'" << std::endl;
            atlas::output::Gmsh(path, atlas::util::Config("coordinates", "lonlat")("ghost", true)).write(mesh);
        }

        if (!meshGeneratorParams.fileXY_.empty()) {
            atlas::output::PathName path(meshGeneratorParams.fileXY_);
            log << "Mesh: writing xy to '" << path << "'" << std::endl;
            atlas::output::Gmsh(path, atlas::util::Config("coordinates", "xy")("ghost", true)).write(mesh);
        }

        if (!meshGeneratorParams.fileXYZ_.empty()) {
            atlas::output::PathName path(meshGeneratorParams.fileXYZ_);
            log << "Mesh: writing xyz to '" << path << "'" << std::endl;
            atlas::output::Gmsh(path, atlas::util::Config("coordinates", "xyz")("ghost", true)).write(mesh);
        }
    }
    catch (...) {
        // Make sure we don't leave an incomplete entry in the cache
        mesh_cache.erase(sign);
        throw;
    }

    mesh_cache.footprint(sign, caching::InMemoryCacheUsage(mesh.footprint(), 0));

    ASSERT(mesh.generated());
    return mesh;
}


void InMemoryMeshCache::print(std::ostream& s) const {
    s << "InMemoryMeshCache[totalFootprint=" << mesh_cache.totalFootprint() << "]";
}


InMemoryMeshCache::InMemoryMeshCache() = default;


}  // namespace caching
}  // namespace mir
