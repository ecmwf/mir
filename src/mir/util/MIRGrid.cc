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

#include <limits>
#include "eckit/log/ResourceUsage.h"
#include "eckit/thread/Mutex.h"
#include "eckit/utils/MD5.h"
#include "atlas/mesh/Elements.h"
#include "atlas/mesh/Mesh.h"
#include "atlas/mesh/Nodes.h"
#include "atlas/mesh/actions/BuildCellCentres.h"
#include "atlas/mesh/actions/BuildXYZField.h"
#include "atlas/output/Gmsh.h"
#include "mir/caching/InMemoryCache.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/AddParallelEdgesConnectivity.h"
#include "mir/util/MIRStatistics.h"


namespace mir {
namespace util {


namespace {
static eckit::Mutex local_mutex;
static InMemoryCache<atlas::Mesh> mesh_cache(
        "mirMesh",
        512 * 1024 * 1024,
        0,
        "$MIR_MESH_CACHE_MEMORY_FOOTPRINT" );
}  // (anonymous namespace)


MIRGrid::MeshGenParams::MeshGenParams() :
    meshGenerator_(""),  // defined by the representation
    meshParallelEdgesConnectivity_(true),
    meshXYZField_(true),
    meshCellCentres_(true),
    file_("") {
    set("three_dimensional", true);
    set("patch_pole",        true);
    set("include_pole",      false);
    set("triangulate",       false);
    set("angle",             0.);
}


MIRGrid::MeshGenParams::MeshGenParams(const std::string& label, const param::MIRParametrisation& param) {

    // use defaults
    *this = MeshGenParams();

//    param.user().get("" + label + "-mesh-add-parallel-edges-connectivity", meshParallelEdgesConnectivity_);
//    param.user().get("" + label + "-mesh-add-field-xyz", meshXYZField_);
//    param.user().get("" + label + "-mesh-add-field-cell-centres", meshCellCentres_);
    param.userParametrisation().get("" + label + "-mesh-generator", meshGenerator_);
    param.userParametrisation().get("" + label + "-mesh-file", file_);
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


void MIRGrid::MeshGenParams::print(std::ostream& s) const {
    s << "MeshGenParams["
      <<  "meshGenerator="                 << meshGenerator_
      << ",meshParallelEdgesConnectivity=" << meshParallelEdgesConnectivity_
      << ",meshXYZField="                  << meshXYZField_
      << ",meshCellCentres="               << meshCellCentres_
      << ",three_dimensional=" << getBool("three_dimensional")
      << ",patch_pole="        << getBool("patch_pole")
      << ",include_pole="      << getBool("include_pole")
      << ",triangulate="       << getBool("triangulate")
      << ",angle="             << getDouble("angle")
      << "]";
}


MIRGrid::MIRGrid(const atlas::Grid& grid) :
    grid_(grid) {
}


MIRGrid::MIRGrid(const MIRGrid& other) {
    grid_ = other.grid_;
    mesh_ = atlas::Mesh();
}


const atlas::Mesh& MIRGrid::mesh(MIRStatistics& statistics, const MeshGenParams& meshGenParams) const {
    ASSERT(!mesh_.generated());
    mesh_ = generateMeshAndCache(statistics, meshGenParams);
    return mesh();
}


const atlas::Mesh& MIRGrid::mesh() const {
    ASSERT(mesh_.generated());
    return mesh_;
}


double MIRGrid::getMeshLongestElementDiagonal() const {
    ASSERT(mesh_.generated());
    using atlas::util::Earth;

    const atlas::mesh::HybridElements::Connectivity& connectivity = mesh_.cells().node_connectivity();
    atlas::array::ArrayView<double, 2> coords = atlas::array::make_view< double, 2 >( mesh_.nodes().field( "xyz" ));
//    atlas::array::ArrayView<double, 2> coords = atlas::array::make_view< double, 2 >( mesh_.nodes().lonlat() );

    // set maximum to Earth radius
    const double dMax = Earth::radiusInMeters();

    size_t firstVirtualPoint = std::numeric_limits<size_t>::max();
    if (mesh_.nodes().metadata().has("NbRealPts")) {
        firstVirtualPoint = mesh_.nodes().metadata().get<size_t>("NbRealPts");
    }

    // assumes:
    // - nb_cols == 3 implies triangle
    // - nb_cols == 4 implies quadrilateral
    // - no other element is supported at this time
    double d = 0.;

    atlas::PointXYZ P[4];
//    atlas::PointLonLat P[4];
    for (size_t e = 0; e < connectivity.rows(); ++e) {
        const size_t nb_cols = connectivity.cols(e);
        ASSERT(nb_cols == 3 || nb_cols == 4);

        // test edges and diagonals (quadrilaterals only)
        // (combinations of ni in [0, nb_cols[ and nj in [ni+1, nb_cols[)
        for (size_t ni = 0; ni < nb_cols; ++ni) {
            const size_t i = size_t(connectivity(e, ni));
            P[ni].assign( coords(i,0), coords(i,1), coords(i,2) );
        }

        for (size_t ni = 0; ni < nb_cols - 1; ++ni) {
            const size_t i = size_t(connectivity(e, ni));
            for (size_t nj = ni + 1; nj < nb_cols; ++nj) {
                const size_t j = size_t(connectivity(e, nj));

                if (i < firstVirtualPoint && j < firstVirtualPoint) {
                    d = std::max(d, Earth::distanceInMeters(P[ni], P[nj]));
                    if (d > dMax) {
                        eckit::Log::warning() << "MIRGrid::getMeshLongestElementDiagonal: limited to maximum " << dMax << "m";
                        return dMax;
                    }
                }

            }
        }
    }

    ASSERT(d > 0.);
    return d;
}


atlas::Mesh MIRGrid::generateMeshAndCache(MIRStatistics& statistics, const MeshGenParams& meshGenParams) const {
    eckit::ResourceUsage usage("Mesh for grid " + grid_.name() + " (" + grid_.uid() + ")", eckit::Log::debug<LibMir>());
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

        const std::string meshGenerator = meshGenParams.meshGenerator_;
        if (meshGenerator.empty()) {
            throw eckit::SeriousBug("MIRGrid::generateMeshAndCache: no mesh generator defined ('" + meshGenerator + "')");
        }

        atlas::MeshGenerator generator(meshGenerator, meshGenParams);
        mesh = generator.generate(grid_);
        ASSERT(mesh.generated());

        // If domain does not include poles, we might need to recover the parallel edges
        // TODO: move to Atlas mesh generator
        atlas::RectangularDomain domain = grid_.domain();
        ASSERT(domain);
        if (meshGenParams.meshParallelEdgesConnectivity_ && !domain.global()) {
            eckit::ResourceUsage usage("AddParallelEdgesConnectivity", eckit::Log::debug<LibMir>());
            eckit::TraceTimer<LibMir> timer("MIRGrid::generateMeshAndCache: AddParallelEdgesConnectivity");
            AddParallelEdgesConnectivity()(mesh, domain.ymax(), domain.ymin());
        }

        // If meshgenerator did not create xyz field already, do it now.
        if (meshGenParams.meshXYZField_) {
            eckit::ResourceUsage usage("BuildXYZField", eckit::Log::debug<LibMir>());
            eckit::TraceTimer<LibMir> timer("MIRGrid::generateMeshAndCache: BuildXYZField");
            atlas::mesh::actions::BuildXYZField()(mesh);
        }

        // Generate barycenters of mesh elements
        if (meshGenParams.meshCellCentres_) {
            eckit::ResourceUsage usage("BuildCellCentres", eckit::Log::debug<LibMir>());
            eckit::TraceTimer<LibMir> timer("MIRGrid::generateMeshAndCache: BuildCellCentres");
            atlas::mesh::actions::BuildCellCentres()(mesh);
        }

        // Some information
        eckit::Log::debug<LibMir>() << "Mesh["
                                        "cells=" << eckit::BigNum(mesh.cells().size())
                                    << ",nodes=" << eckit::BigNum(mesh.nodes().size())
                                    << "," << meshGenParams
                                    << "]"
                                    << std::endl;

        // Write file
        if (!meshGenParams.file_.empty()) {
            atlas::output::PathName path(meshGenParams.file_);

            eckit::Log::debug<LibMir>() << "Mesh: writing to '" << path << "'" << std::endl;
            atlas::output::Gmsh(path, atlas::util::Config("coordinates", "xyz")).write(mesh);
        }

    }
    catch (...) {
        // Make sure we don't leave an incomplete entry in the cache
        mesh_cache.erase(md5);
        throw;
    }

    mesh_cache.footprint(md5, InMemoryCacheUsage(mesh.footprint(), 0) ) ;

    return mesh;
}


void MIRGrid::hash(eckit::MD5& md5) const {
    md5 << grid_;
    if (mesh_.generated()) {
        md5 << meshGenParams_;
    }
}


}  // namespace util
}  // namespace mir
