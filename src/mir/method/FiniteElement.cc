/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date May 2015

#include "mir/method/FiniteElement.h"

#include <algorithm>

#include "eckit/config/Resource.h"
#include "eckit/log/BigNum.h"
#include "eckit/log/ETA.h"
#include "eckit/log/Plural.h"
#include "eckit/log/Seconds.h"
#include "eckit/log/Timer.h"

#include "atlas/actions/BuildXYZField.h"
#include "atlas/geometry/QuadrilateralIntersection.h"
#include "atlas/geometry/Ray.h"
#include "atlas/geometry/TriangleIntersection.h"
#include "atlas/meshgen/Delaunay.h"
#include "atlas/grids/ReducedGrid.h"
#include "atlas/util/IndexView.h"
#include "atlas/io/Gmsh.h"
#include "atlas/actions/BuildXYZField.h"
#include "atlas/actions/BuildCellCentres.h"

#include "mir/param/MIRParametrisation.h"

namespace mir {
namespace method {

FiniteElement::MeshGenParams::MeshGenParams() {
    set("three_dimensional", true);
    set("patch_pole",        true);
    set("include_pole",      false);
    set("angle",             0.);
    set("triangulate",       false);
}


FiniteElement::FiniteElement(const param::MIRParametrisation &param) :
    MethodWeighted(param) {
}


FiniteElement::~FiniteElement() {
}

void FiniteElement::hash( eckit::MD5 &md5) const {
    MethodWeighted::hash(md5);
}

namespace {

struct MeshStats {

    size_t nb_triags;
    size_t nb_quads;
    size_t inp_npts;
    size_t out_npts;

    size_t size() const {
        return nb_triags + nb_quads;
    }

    void print(std::ostream &s) const {
        s << "MeshStats[nb_triags=" << eckit::BigNum(nb_triags)
          << ",nb_quads=" << eckit::BigNum(nb_quads)
          << ",inp_npts=" << eckit::BigNum(inp_npts)
          << ",out_npts=" << eckit::BigNum(out_npts) << "]";
    }

    friend std::ostream &operator<<(std::ostream &s, const MeshStats &p) {
        p.print(s);
        return s;
    }
};

}


/// Finds in which element the point is contained by projecting the point with each nearest element

static bool projectPointToElements(const MeshStats &stats,
                                   const atlas::ArrayView<double, 2> &icoords,
                                   const atlas::IndexView<int,    2> &triag_nodes,
                                   const atlas::IndexView<int,    2> &quads_nodes,
                                   const FiniteElement::Point &p,
                                   std::vector< WeightMatrix::Triplet > &weights_triplets,
                                   size_t ip,
                                   atlas::ElemIndex3::NodeList::const_iterator start,
                                   atlas::ElemIndex3::NodeList::const_iterator finish ) {

    size_t idx [4];
    double w[4];

    atlas::geometry::Ray ray( p.data() );

    for (atlas::ElemIndex3::NodeList::const_iterator itc = start; itc != finish; ++itc) {

        atlas::ElemPayload elem = (*itc).value().payload();

        if ( elem.type_ == 't') { /* triags */

            const size_t &tid = elem.id_;

            ASSERT( tid < stats.nb_triags );

            idx[0] = triag_nodes(tid, 0);
            idx[1] = triag_nodes(tid, 1);
            idx[2] = triag_nodes(tid, 2);

            ASSERT( idx[0] < stats.inp_npts && idx[1] < stats.inp_npts && idx[2] < stats.inp_npts );

            atlas::geometry::TriangleIntersection triag(
                icoords[idx[0]].data(),
                icoords[idx[1]].data(),
                icoords[idx[2]].data());

            atlas::geometry::Intersect is = triag.intersects(ray);

            if (is) {

                // weights are the linear Lagrange function evaluated at u,v (aka baricentric coordinates)
                w[0] = 1. - is.u - is.v;
                w[1] = is.u;
                w[2] = is.v;

                for (size_t i = 0; i < 3; ++i)
                    weights_triplets.push_back( WeightMatrix::Triplet( ip, idx[i], w[i] ) );

                return true;
            }

        } else {  /* quads */

            ASSERT(elem.type_ == 'q');

            const size_t &qid = elem.id_;

            ASSERT( qid < stats.nb_quads );

            idx[0] = quads_nodes(qid, 0);
            idx[1] = quads_nodes(qid, 1);
            idx[2] = quads_nodes(qid, 2);
            idx[3] = quads_nodes(qid, 3);

            ASSERT( idx[0] < stats.inp_npts && idx[1] < stats.inp_npts &&
                    idx[2] < stats.inp_npts && idx[3] < stats.inp_npts );

            atlas::geometry::QuadrilateralIntersection quad(
                icoords[idx[0]].data(),
                icoords[idx[1]].data(),
                icoords[idx[2]].data(),
                icoords[idx[3]].data() );

            if ( !quad.validate() ) { // somewhat expensive sanity check
                eckit::Log::warning() << "Invalid Quad : " << quad << std::endl;
                throw eckit::SeriousBug("Found invalid quadrilateral in mesh", Here());
            }

            atlas::geometry::Intersect is = quad.intersects(ray);

            if (is) {

                // weights are the bilinear Lagrange function evaluated at u,v
                w[0] = (1. - is.u) * (1. - is.v);
                w[1] =       is.u  * (1. - is.v);
                w[2] =       is.u  *       is.v ;
                w[3] = (1. - is.u) *       is.v ;

                for (size_t i = 0; i < 4; ++i)
                    weights_triplets.push_back( WeightMatrix::Triplet( ip, idx[i], w[i] ) );

                return true;

            }
        }

    } // loop over nearest elements

    ASSERT(start != finish);

    return false;
}


static const double maxFractionElemsToTry = 0.2; // try to project to 20% of total number elements before giving up

void FiniteElement::assemble(WeightMatrix &W, const atlas::Grid &in, const atlas::Grid &out) const {

    // FIXME: arguments
    eckit::Log::info() << "FiniteElement::assemble" << std::endl;

    eckit::Log::info() << "  Input  Grid: " << in  << std::endl;
    eckit::Log::info() << "  Output Grid: " << out << std::endl;

    const atlas::Domain &inDomain = in.domain();

    atlas::Mesh &i_mesh = in.mesh();
    atlas::Mesh &o_mesh = out.mesh();

    eckit::Log::info() << "  Input  Mesh: " << i_mesh << std::endl;
    eckit::Log::info() << "  Output Mesh: " << o_mesh << std::endl;

    eckit::Timer timer("Compute weights");

    // FIXME: using the name() is not the right thing, although it should work, but create too many cached meshes.
    // We need to use the mesh-generator
    {
        eckit::Timer timer("Generate mesh");
        generateMesh(in, i_mesh);

        static bool dumpMesh = eckit::Resource<bool>("$MIR_DUMP_MESH", false);
        if (dumpMesh) {
            atlas::io::Gmsh gmsh;
            gmsh.options.set<std::string>("nodes", "xyz");

            eckit::Log::info() << "Dumping input mesh to input.msh" << std::endl;
            gmsh.write(i_mesh, "input.msh");

            eckit::Log::info() << "Dumping output mesh to output.msh" << std::endl;
            atlas::actions::BuildXYZField("xyz")(o_mesh);
            gmsh.write(o_mesh, "output.msh");
        }
    }

    // generate baricenters of each triangle & insert the baricenters on a kd-tree
    {
        eckit::Timer timer("Tesselation::create_cell_centres");
        atlas::actions::BuildCellCentres()(i_mesh);
    }

    eckit::ScopedPtr<atlas::ElemIndex3> eTree;
    {
        eckit::Timer timer("create_element_centre_index");
        eTree.reset( create_element_centre_index(i_mesh) );
    }

    // input mesh

    atlas::FunctionSpace  &i_nodes  = i_mesh.function_space( "nodes" );
    atlas::ArrayView<double, 2> icoords  ( i_nodes.field( "xyz" ));

    atlas::FunctionSpace &triags = i_mesh.function_space( "triags" );
    atlas::IndexView<int, 2> triag_nodes ( triags.field( "nodes" ) );

    atlas::FunctionSpace &quads = i_mesh.function_space( "quads" );
    atlas::IndexView<int, 2> quads_nodes ( quads.field( "nodes" ) );

    // output mesh

    // In case xyz field in the out mesh, build it
    atlas::actions::BuildXYZField("xyz")(out.mesh());

    atlas::FunctionSpace  &o_nodes  = o_mesh.function_space( "nodes" );
    atlas::ArrayView<double, 2> ocoords ( o_nodes.field( "xyz" ) );
    atlas::ArrayView<double, 2> olonlat ( o_nodes.field( "lonlat" ));

    MeshStats stats;
    stats.nb_triags = triags.shape(0);
    stats.nb_quads  = quads.shape(0);
    stats.inp_npts  = i_nodes.shape(0);
    stats.out_npts  = o_nodes.shape(0);

    eckit::Log::info() << stats << std::endl;

    // weights -- one per vertice of element, triangles (3) or quads (4)

    std::vector< WeightMatrix::Triplet > weights_triplets; // structure to fill-in sparse matrix
    weights_triplets.reserve( stats.out_npts * 4 );         // preallocate space as if all elements where quads

    // search nearest k cell centres

    const size_t maxNbElemsToTry = std::max<size_t>(64, stats.size() * maxFractionElemsToTry);
    size_t max_neighbours = 0;

    eckit::Log::info() << "Projecting " << eckit::Plural(stats.out_npts, "output point") << " to input mesh " << in.shortName() << std::endl;

    {
        eckit::Timer timerProj("Projecting");

        for ( size_t ip = 0; ip < stats.out_npts; ++ip ) {

            if (ip && (ip % 10000 == 0)) {
                double rate = ip / timerProj.elapsed();
                eckit::Log::info() << eckit::BigNum(ip) << " ..."  << eckit::Seconds(timerProj.elapsed())
                                   << ", rate: " << rate << " points/s, ETA: "
                                   << eckit::ETA( (stats.out_npts - ip) / rate )
                                   << std::endl;
            }

            if (!inDomain.contains(olonlat[ip][atlas::LON], olonlat[ip][atlas::LAT])) {
                continue;
            }

            Point p ( ocoords[ip].data() ); // lookup point

            size_t kpts = 1;
            bool success = false;
            while (!success && kpts <= maxNbElemsToTry) {

                max_neighbours = std::max(kpts, max_neighbours);

                atlas::ElemIndex3::NodeList cs = eTree->kNearestNeighbours(p, kpts);
                success = projectPointToElements(stats,
                                                 icoords,
                                                 triag_nodes,
                                                 quads_nodes,
                                                 p,
                                                 weights_triplets,
                                                 ip,
                                                 cs.begin(),
                                                 cs.end() );

                kpts *= 2;

            }

            if (!success) {
                // If this fails, consider lowering atlas::grid::parametricEpsilon
                eckit::Log::info() << "Failed to project point " << ip << " " << p
                                   << " with coordinates lon=" << olonlat[ip][atlas::LON] << ", lat=" << olonlat[ip][atlas::LAT]
                                   << " after " << eckit::Plural(kpts, "attempt") << std::endl;
                throw eckit::SeriousBug("Could not project point");
            }
        }
    }

    eckit::Log::info() << "Projected " << eckit::Plural(stats.out_npts, "point") << std::endl;
    eckit::Log::info() << "Maximum neighbours searched was " << eckit::Plural(max_neighbours, "element") << std::endl;

    W.setFromTriplets(weights_triplets); // fill sparse matrix
}


void FiniteElement::generateMesh(const atlas::Grid &grid, atlas::Mesh &mesh) const {

    ///  This raises another issue: how to cache meshes generated with different parametrisations?
    ///  We must md5 the MeshGenerator itself.

    std::string meshgenerator("ReducedGrid");             // Fastest option available by default

    parametrisation_.get("meshgenerator", meshgenerator); // Override with MIRParametrisation

    const atlas::grids::ReducedGrid *reduced = dynamic_cast<const atlas::grids::ReducedGrid *>(&grid);

    // Falling back to "Delaunay" if the mesh is not a ReducedGrid
    if (reduced == 0 && meshgenerator == "ReducedGrid") {
        meshgenerator = "Delaunay";
    }

    if ( reduced )
        eckit::Log::info() << "Mesh is ReducedGrid " << grid.shortName() << '\n';

    eckit::ScopedPtr<atlas::meshgen::MeshGenerator> generator( atlas::meshgen::MeshGeneratorFactory::build(meshgenerator, meshgenparams_) );
    generator->generate(*reduced, mesh);

    // If meshgenerator did not create xyz field already, do it now.
    atlas::actions::BuildXYZField()(mesh);
}


}  // namespace method
}  // namespace mir

