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

#include "eckit/geometry/Point3.h"
#include "eckit/log/ResourceUsage.h"
#include "eckit/thread/Mutex.h"
#include "eckit/utils/MD5.h"
#include "atlas/array.h"
#include "atlas/mesh/actions/BuildXYZField.h"
#include "atlas/mesh/Mesh.h"
#include "mir/caching/InMemoryCache.h"
#include "mir/config/LibMir.h"
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
    set("angle",             0.);
    set("triangulate",       false);
}


MIRGrid::MIRGrid(const atlas::Grid& grid, const Domain& domain, const MeshGenParams& meshGenParams) :
    grid_(grid),
    domain_(domain),
    meshGenParams_(meshGenParams),
    coordsXYZ_(0) {
}


MIRGrid::MIRGrid(const MIRGrid& other) {
    operator=(other);
}


MIRGrid& MIRGrid::operator=(const MIRGrid& other) {
    const_cast<atlas::Grid&>(grid_)            = other.grid_;
    const_cast<Domain&>(domain_)               = other.domain_;
    const_cast<MeshGenParams&>(meshGenParams_) = other.meshGenParams_;

    mesh_ = atlas::Mesh();
    coordsXYZ_.reset();

    return *this;
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

    MIRStatistics dummy; // TODO: use the global one
    InMemoryCacheUser<atlas::Mesh> cache_use(mesh_cache, dummy.meshCache_ /*statistics.meshCache_*/);

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


const atlas::array::Array& MIRGrid::coordsXYZ() const {
    using namespace atlas::array;

    if (!coordsXYZ_) {
        coordsXYZ_.reset(Array::create< double >(grid_.size(), 3));
        ArrayView< double, 2 > xyz = make_view< double, 2 >(*coordsXYZ_);

        size_t i = 0;
        for (atlas::PointLonLat p : grid_.lonlat()) {
            atlas::PointXYZ x = atlas::lonlat_to_geocentric(p);
            xyz(i, 0) = x.x();
            xyz(i, 1) = x.y();
            xyz(i, 2) = x.z();
            ++i;
        }
    }

    return *coordsXYZ_;
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
