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




#include "eckit/log/ResourceUsage.h"
#include "eckit/thread/Mutex.h"
#include "eckit/utils/MD5.h"
#include "atlas/mesh/actions/BuildXYZField.h"
#include "atlas/mesh/Mesh.h"
#include "mir/caching/InMemoryCache.h"
#include "mir/config/LibMir.h"
#include "mir/util/MIRStatistics.h"
#include "mir/util/MIRGrid.h"


namespace mir {
namespace util {


namespace {
static util::MIRStatistics dummyStatistics;
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
    set("angle",             0.);
    set("triangulate",       false);
}



MIRGrid::MIRGrid(const atlas::Grid& grid, const Domain& domain) :
    grid_(grid),
    domain_(domain),
    statistics_(dummyStatistics) {
}


MIRGrid::MIRGrid(const atlas::Grid& grid, const Domain& domain, util::MIRStatistics& statistics, const MeshGenParams& meshGenParams) :
    grid_(grid),
    domain_(domain),
    meshGenParams_(meshGenParams),
    statistics_(statistics) {
}


MIRGrid::MIRGrid(const MIRGrid& other) :
    statistics_(other.statistics_) {

    grid_   = other.grid_;
    domain_ = other.domain_;

    meshGenParams_ = other.meshGenParams_;

    mesh_ = atlas::Mesh();
}


MIRGrid::operator const atlas::Grid&() const {
    return grid_;
}


const Domain& MIRGrid::domain() const {
    return domain_;
}


atlas::Mesh& MIRGrid::mesh() const {
    if (!mesh_.generated()) {
        mesh_ = generateMeshAndCache();
        ASSERT(mesh_.generated());
    }
    return mesh_;
}


atlas::Mesh MIRGrid::generateMeshAndCache() const {
    eckit::ResourceUsage usage("MESH for " + std::to_string(grid_));
    InMemoryCacheUser<atlas::Mesh> cache_use(mesh_cache, statistics_.meshCache_);

    eckit::MD5 md5;
    hash(md5);

    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    InMemoryCache<atlas::Mesh>::iterator j = mesh_cache.find(md5);
    if (j != mesh_cache.end()) {
        return *j;
    }

    atlas::Mesh& mesh = mesh_cache[md5];
    try {

        atlas::MeshGenerator generator(meshGenParams_.meshGenerator_, meshGenParams_);
        mesh = generator.generate(grid_);

        // If meshgenerator did not create xyz field already, do it now.
        atlas::mesh::actions::BuildXYZField()(mesh);

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
    // TODO: make MeshGenParams::hash
    md5 << grid_ << domain_ << meshGenParams_ << meshGenParams_.meshGenerator_;
}


size_t MIRGrid::size() const {
    return grid_.size();
}


}  // namespace util
}  // namespace mir
