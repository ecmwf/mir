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
#include <iostream>
#include <string>

#include "eckit/log/Timer.h"
#include "eckit/log/Plural.h"
#include "eckit/log/BigNum.h"
#include "eckit/log/Seconds.h"
#include "eckit/log/ETA.h"
#include "eckit/config/Resource.h"

#include "eckit/types/Types.h"

#include "atlas/Tesselation.h"
#include "atlas/util/IndexView.h"

#include "atlas/PointIndex3.h"
#include "atlas/geometry/Ray.h"
#include "atlas/geometry/TriangleIntersection.h"
#include "atlas/geometry/QuadrilateralIntersection.h"
#include "atlas/meshgen/MeshGenerator.h"
#include "atlas/MeshCache.h"
#include "atlas/meshgen/ReducedGridMeshGenerator.h"
#include "atlas/meshgen/Delaunay.h"
#include "atlas/grids/ReducedGrid.h"

#include "mir/param/MIRParametrisation.h"

using namespace eckit;

using atlas::Grid;
using atlas::Mesh;
using atlas::MeshCache;
using atlas::FunctionSpace;
using atlas::IndexView;
using atlas::ArrayView;
using atlas::ElemPayload;
using atlas::ElemIndex3;
using atlas::Tesselation;
using atlas::create_element_centre_index;
using atlas::geometry::Intersect;
using atlas::geometry::TriangleIntersection;
using atlas::geometry::QuadrilateralIntersection;
using atlas::geometry::Ray;
using atlas::meshgen::MeshGenerator;
using atlas::meshgen::MeshGeneratorFactory;
using atlas::grids::ReducedGrid;

#define TEST_FINITE_ELEMENT

namespace mir {
namespace method {

FiniteElement::FiniteElement(const param::MIRParametrisation &param) :
    MethodWeighted(param) {
}


FiniteElement::~FiniteElement() {
}


const char *FiniteElement::name() const {
    return "finite-element";
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

    size_t nbElems() const {
        return nb_triags + nb_quads;
    }

    void print(std::ostream& s) const {
        s << "MeshStats[nb_triags=" << nb_triags
          << ",nb_quads=" << nb_quads
          << ",inp_npts=" << inp_npts
          << ",out_npts=" << out_npts << "]";
    }

    friend std::ostream& operator<<(std::ostream& s, const MeshStats& p) {
        p.print(s);
        return s;
    }
};

}

void FiniteElement::generateMesh(const Grid& g, Mesh& mesh) const
{
    std::string uid = g.unique_id();

    MeshCache cache;

    if (cache.retrieve(g, mesh)) return;

    std::cout << "Mesh not in cache -- tesselating grid " << uid << std::endl;

    /// @TODO Ask Baudouin best way to build and parametrize the mesh generator
    ///       MeshGenerator is in Atlas -- should we bring to MIR ??
    ///       If stays in Atlas, we cannot pass MirParametrisation
    ///
    ///  We should be using something like:
    ///
    //    std::string meshGenerator;
    //    ASSERT(parametrisation_.get("meshGenerator", meshGenerator));
    //    eckit::ScopedPtr<MeshGenerator> meshGen( MeshGeneratorFactory::build(meshGenerator) );
    //    meshGen->tesselate(in, i_mesh);

    bool mirReducedGridMG = eckit::Resource<bool>("$MIR_REDUCED_GRID_MG", false);

    const atlas::grids::ReducedGrid* rg = dynamic_cast<const atlas::grids::ReducedGrid*>(&g);
    if (mirReducedGridMG && rg) {

      // fast tesselation method, specific for ReducedGrid's

      std::cout << "Mesh is ReducedGrid " << g.shortName() << std::endl;

      ASSERT(rg);

      atlas::meshgen::ReducedGridMeshGenerator mg;

      bool mirReducedGridMGSplitQuads = eckit::Resource<bool>("$MIR_REDUCED_GRID_MG_SPLIT_QUADS", false);

      // force these flags
      mg.options.set("three_dimensional",true);
      mg.options.set("patch_pole",true);
      mg.options.set("include_pole",false);
      mg.options.set("triangulate",mirReducedGridMGSplitQuads);

      mg.generate(*rg, mesh);

    } else {

      // slower, more robust tesselation method, using Delaunay triangulation

      std::cout << "Using Delaunay triangulation on grid: " << g.shortName() << std::endl;

      atlas::meshgen::Delaunay mg;
      mg.tesselate(g, mesh);
    }

    cache.insert(g, mesh);
}

bool projectPointToElements(const MeshStats& stats,
                            const ArrayView<double, 2>& icoords,
                            const IndexView<int,    2>& triag_nodes,
                            const IndexView<int,    2>& quads_nodes,
                            FiniteElement::Point& p,
                            std::vector< Eigen::Triplet<double> >& weights_triplets,
                            size_t ip,
                            atlas::ElemIndex3::NodeList::const_iterator start,
                            atlas::ElemIndex3::NodeList::const_iterator finish ) {

    int idx [4];
    double w[4];

    // find in which element the point is contained
    // by computing the intercetion of the point with each nearest triangle

    Ray ray( p.data() );

    atlas::ElemIndex3::NodeList::const_iterator itc = start;
    for ( ; itc != finish; ++itc ) {

        ElemPayload elem = (*itc).value().payload();

        if ( elem.type_ == 't') { /* triags */

            const size_t &tid = elem.id_;

            ASSERT( tid < stats.nb_triags );

            idx[0] = triag_nodes(tid, 0);
            idx[1] = triag_nodes(tid, 1);
            idx[2] = triag_nodes(tid, 2);

            ASSERT( idx[0] < stats.inp_npts && idx[1] < stats.inp_npts && idx[2] < stats.inp_npts );

            TriangleIntersection triag(icoords[idx[0]].data(),
                                       icoords[idx[1]].data(),
                                       icoords[idx[2]].data());

            Intersect is = triag.intersects(ray);

            //            Log::info() << is << std::endl;

            // weights are the linear Lagrange function evaluated at u,v (aka baricentric coordinates)
            if (is) {
                w[0] = 1. - is.u - is.v;
                w[1] = is.u;
                w[2] = is.v;

                for (int i = 0; i < 3; ++i)
                    weights_triplets.push_back( Eigen::Triplet<double>( ip, idx[i], w[i] ) );

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

            QuadrilateralIntersection quad( icoords[idx[0]].data(),
                                            icoords[idx[1]].data(),
                                            icoords[idx[2]].data(),
                                            icoords[idx[3]].data() );

            // this check is somewhat expensive but is better to keep it for sanity
            if( !quad.validate() )
            {
                Log::warning() << "Invalid Quad : " << quad << std::endl;
                throw SeriousBug("Found invalid quadrilateral in mesh", Here());
            }

#ifdef TEST_FINITE_ELEMENT
            Intersect is = quad.intersectsTG(ray);
#else
            Intersect is = quad.intersects(ray);
#endif

            // weights are the bilinear Lagrange function evaluated at u,v
            if (is) {

#ifdef TEST_FINITE_ELEMENT // VERY EXPENSIVE -- DEBUG ONLY
                if( !quad.validateIntersection(ray) )
                    throw SeriousBug("Point projects to quad but not to its sub-triangles", Here());
#endif

                w[0] = (1. - is.u) * (1. - is.v);
                w[1] =       is.u  * (1. - is.v);
                w[2] =       is.u  *       is.v ;
                w[3] = (1. - is.u) *       is.v ;

                for (int i = 0; i < 4; ++i)
                    weights_triplets.push_back( Eigen::Triplet<double>( ip, idx[i], w[i] ) );

                return true;

            }
#ifdef TEST_FINITE_ELEMENT // VERY EXPENSIVE -- DEBUG ONLY
            else {
                if(quad.validateIntersection(ray)) {
                    Log::warning() << "Point " << ip << ":" << p << " "
                                   << "Quad"   << quad
                                   << std::endl;
                    throw SeriousBug("Point projects to sub-triangles but not to quad", Here());
                }
            }
#endif

        }

    } // loop over nearest elements

    return false;
}

void FiniteElement::assemble(WeightMatrix& W, const Grid &in, const Grid& out) const {

    // FIXME arguments:
    eckit::Log::info() << "FiniteElement::assemble" << std::endl;

    Mesh &i_mesh = const_cast<Mesh &>(in.mesh());  // we modify the mesh when we tesselate
    Mesh &o_mesh = const_cast<Mesh &>(out.mesh());

    eckit::Timer timer("Compute weights");

    generateMesh(in, i_mesh);

    // generate baricenters of each triangle & insert the baricenters on a kd-tree

    {
        eckit::Timer timer("Tesselation::create_cell_centres");
        Tesselation::create_cell_centres(i_mesh);
    }

    eckit::ScopedPtr<atlas::ElemIndex3> eTree;
    {
        eckit::Timer timer("create_element_centre_index");
        eTree.reset( create_element_centre_index(i_mesh) );
    }

    // input mesh

    FunctionSpace  &i_nodes  = i_mesh.function_space( "nodes" );
    ArrayView<double, 2> icoords  ( i_nodes.field<double>( "xyz" ));

    FunctionSpace &triags = i_mesh.function_space( "triags" );
    IndexView<int, 2> triag_nodes ( triags.field<int>( "nodes" ) );

    FunctionSpace &quads = i_mesh.function_space( "quads" );
    IndexView<int, 2> quads_nodes ( quads.field<int>( "nodes" ) );

    // output mesh

    FunctionSpace  &o_nodes  = o_mesh.function_space( "nodes" );
    ArrayView<double, 2> ocoords ( o_nodes.field( "xyz" ) );


    MeshStats stats;

    stats.nb_triags = triags.shape(0);
    stats.nb_quads  = quads.shape(0);
    stats.inp_npts  = i_nodes.shape(0);
    stats.out_npts  = o_nodes.shape(0);

    Log::info() << "Mesh has " << eckit::Plural(stats.nb_triags, "triangle")
                << " and " << eckit::Plural(stats.nb_quads, "quadrilateral") << std::endl;
    Log::info() << stats << std::endl;

    // weights -- one per vertice of element, triangles (3) or quads (4)

    std::vector< Eigen::Triplet<double> > weights_triplets; /* structure to fill-in sparse matrix */

    weights_triplets.reserve( stats.out_npts * 4 );

    /* search nearest k cell centres */

    size_t max_neighbours = 0;

    Log::info() << "Projecting " << stats.out_npts << " output points to input mesh " << in.shortName() << std::endl;

    std::vector<Point> failed_;

    {
        eckit::Timer timerProj("Projecting");

        for ( size_t ip = 0; ip < stats.out_npts; ++ip ) {

            if (ip && (ip % 100000 == 0)) {
                double rate = ip / timerProj.elapsed();
                Log::info() << eckit::BigNum(ip) << " ..."  << eckit::Seconds(timerProj.elapsed())
                            << ", rate: " << rate << " points/s, ETA: "
                            << eckit::ETA( (stats.out_npts - ip) / rate )
                            << std::endl;
            }

            Point p ( ocoords[ip].data() ); // lookup point

            size_t done = 0;
            size_t kpts = 1;
            bool success = false;

            do {
                if(done >= stats.nbElems()) {
                    failed_.push_back(p);
                    Log::warning() << "Point " << ip << " with coords " << p << " failed projection ..." << std::endl;
                    break;
                }

                ElemIndex3::NodeList cs = eTree->kNearestNeighbours(p, kpts);

                success = projectPointToElements(stats,
                                                 icoords,
                                                 triag_nodes,
                                                 quads_nodes,
                                                 p,
                                                 weights_triplets,
                                                 ip,
                                                 cs.begin()+done,
                                                 cs.end() );

                done = kpts;
                kpts = std::min(4*done,stats.nbElems()); // increase the number of searched elements
            } while( !success );

            max_neighbours = std::max(done, max_neighbours);

//            Log::info() << "Visited " << done << " elements"  << std::endl;
        }
    }

    Log::info() << "Projected " << stats.out_npts - failed_.size() << " points"  << std::endl;
    Log::info() << "Maximum neighbours searched " << max_neighbours << " elements"  << std::endl;

//    if (failed_.size()) {
//        std::ostringstream os;
//        os << "Failed to project following points into input Grid " << in.shortName() << ":" << std::endl;
//        for (size_t i = 0; i < failed_.size(); ++i)
//            os << failed_[i] << std::endl;
//        throw SeriousBug(os.str(), Here());
//    }

    // fill-in sparse matrix

    W.setFromTriplets(weights_triplets.begin(), weights_triplets.end());
}


void FiniteElement::print(std::ostream &out) const {
    out << "FiniteElement[]";
}


namespace {
static MethodBuilder< FiniteElement > __finiteelement("finite-element");
}

}  // namespace method
}  // namespace mir

