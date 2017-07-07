/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date   May 2016


#include "mir/util/MIRGrid.h"

#include "eckit/log/ResourceUsage.h"
#include "eckit/thread/Mutex.h"
#include "eckit/utils/MD5.h"
#include "atlas/mesh/Mesh.h"
#include "atlas/mesh/actions/BuildCellCentres.h"
#include "atlas/mesh/actions/BuildXYZField.h"
#include "atlas/output/Gmsh.h"
#include "mir/caching/InMemoryCache.h"
#include "mir/config/LibMir.h"
#include "mir/method/AddParallelEdgesConnectivity.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/MIRStatistics.h"


namespace mir {
namespace util {


namespace {
static eckit::Mutex local_mutex;
static InMemoryCache<atlas::Mesh> mesh_cache(
        "mirMesh",
        512 * 1024 * 1024,
        "$MIR_MESH_CACHE_MEMORY_FOOTPRINT" );
}  // (anonymous namespace)


MIRGrid::MeshGenParams::MeshGenParams() {
    set("three_dimensional", true);
    set("patch_pole",        true);
    set("include_pole",      false);
    set("triangulate",       false);
    set("angle",             0.);

    meshGenerator_ = "structured";
    meshParallelEdgesConnectivity_ = true;
    meshXYZField_ = true;
    meshCellCentres_ = true;
    dump_ = "";
}


MIRGrid::MeshGenParams::MeshGenParams(const std::string& label, const param::MIRParametrisation& param) {

    *this = MeshGenParams();

//    param.get(label + "-mesh-add-parallel-edges-connectivity", meshParallelEdgesConnectivity_);
//    param.get(label + "-mesh-add-field-xyz", meshXYZField_);
//    param.get(label + "-mesh-add-field-cell-centres", meshCellCentres_);
    param.get(label + "-mesh-generator", meshGenerator_);
    param.get(label + "-mesh-dump", dump_);
}


void MIRGrid::MeshGenParams::hash(eckit::MD5& md5) const {

    for (const char* p : {"three_dimensional", "patch_pole", "include_pole", "triangulate"}) {
        bool value = false;
        ASSERT(get(p, value));
        md5 << value;
    }

    double angle = 0.;
    ASSERT(get("angle", angle));

    md5 << angle
        << meshGenerator_
        << meshParallelEdgesConnectivity_
        << meshXYZField_
        << meshCellCentres_;
}


MIRGrid::MIRGrid(const atlas::Grid& grid) :
    grid_(grid) {
}


MIRGrid::MIRGrid(const MIRGrid& other) {
    grid_ = other.grid_;
    mesh_ = atlas::Mesh();
}


MIRGrid::operator const atlas::Grid&() const {
    return grid_;
}


const atlas::Mesh& MIRGrid::mesh(util::MIRStatistics& statistics, const MeshGenParams& meshGenParams) const {
    ASSERT(!mesh_.generated());
    mesh_ = generateMeshAndCache(statistics, meshGenParams);
    return mesh();
}


const atlas::Mesh& MIRGrid::mesh() const {
    ASSERT(mesh_.generated());
    return mesh_;
}


atlas::Mesh MIRGrid::generateMeshAndCache(util::MIRStatistics& statistics, const MeshGenParams& meshGenParams) const {
    eckit::ResourceUsage usage("MESH for " + std::to_string(grid_));
    InMemoryCacheUser<atlas::Mesh> cache_use(mesh_cache, statistics.meshCache_);

    // generate signature including the mesh generation settings
    eckit::MD5 md5;
    meshGenParams_ = meshGenParams;
    hash(md5);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    InMemoryCache<atlas::Mesh>::iterator j = mesh_cache.find(md5);
    if (j != mesh_cache.end()) {
        return *j;
    }

    atlas::Mesh& mesh = mesh_cache[md5];
    try {

        atlas::MeshGenerator generator(meshGenParams.meshGenerator_, meshGenParams);
        mesh = generator.generate(grid_);
        ASSERT(mesh.generated());

        // If meshgenerator did not create xyz field already, do it now.
        if (meshGenParams.meshXYZField_) {
            eckit::ResourceUsage usage("BuildXYZField");
            eckit::TraceTimer<LibMir> timer("MIRGrid::generateMeshAndCache: BuildXYZField");
            atlas::mesh::actions::BuildXYZField()(mesh);
        }

        // If domain does not include poles, we might need to recover the parallel edges
        // TODO: move to Atlas mesh generator
        atlas::Domain domain = grid_.domain();
        if (meshGenParams.meshParallelEdgesConnectivity_ && !domain.global()) {
            eckit::ResourceUsage usage("AddParallelEdgesConnectivity");
            eckit::TraceTimer<LibMir> timer("MIRGrid::generateMeshAndCache: AddParallelEdgesConnectivity");

            // FIXME hacky way to extract North/South parallels
            atlas::Domain::Spec spec(domain.spec());
            double ymin = 0.;
            double ymax = 0.;
            ASSERT(spec.get("ymin", ymin));
            ASSERT(spec.get("ymax", ymax));

            ASSERT(-90. <= ymin && ymin < ymax && ymax <= 90.);
            method::AddParallelEdgesConnectivity()(mesh, ymax, ymin);
        }

        // Generate barycenters of mesh elements
        if (meshGenParams.meshCellCentres_) {
            eckit::ResourceUsage usage("BuildCellCentres");
            eckit::TraceTimer<LibMir> timer("MIRGrid::generateMeshAndCache: BuildCellCentres");
            atlas::mesh::actions::BuildCellCentres()(mesh);
        }

        // Dump
        if (!meshGenParams.dump_.empty()) {
            atlas::output::PathName path(meshGenParams.dump_);

            eckit::Log::debug<LibMir>() << "Dumping mesh to '" << path << "'" << std::endl;
            atlas::output::Gmsh(path, atlas::util::Config("coordinates", "xyz")).write(mesh_);
        }

    }
    catch (...) {
        // Make sure we don't leave an incomplete entry in the cache
        mesh_cache.erase(md5);
        throw;
    }

    mesh_cache.footprint(md5, mesh.footprint());

    return mesh;
}


void MIRGrid::hash(eckit::MD5& md5) const {
    md5 << grid_;
    if (mesh_.generated()) {
        md5 << meshGenParams_;
    }
}


size_t MIRGrid::size() const {
    return grid_.size();
}


}  // namespace util
}  // namespace mir
