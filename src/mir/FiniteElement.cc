/*
 * (C) Copyright 1996-2014 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <string>

#include <boost/progress.hpp>

#include "eckit/log/Timer.h"

#include "atlas/grid/Tesselation.h"
#include "atlas/grid/PointIndex3.h"

#include "mir/FiniteElement.h"

//------------------------------------------------------------------------------------------------------

using namespace Eigen;
using namespace eckit;
using namespace atlas;
using namespace atlas::grid;

namespace mir {

//------------------------------------------------------------------------------------------------------

//#define DUMP_PROJ
#ifdef DUMP_PROJ
    static std::ofstream of("found.txt");
#endif

bool FiniteElement::project_point_to_triangle(  Point& p, Vector3d& phi, int idx[3], const size_t k ) const
{
    bool found = false;

    FieldT<int>& triag_nodes = *ptriag_nodes;
    FieldT<double>& icoords = *picoords;

    PointIndex3::NodeList cs = ptree->kNearestNeighbours(p,k);

#if 0
    std::cout << p << std::endl;
    for( size_t i = 0; i < cs.size(); ++i )
    {
        std::cout << cs[i] << std::endl;
    }
#endif

    // find in which triangle the point is contained
    // by computing the intercetion of the point with each nearest triangle

    Isect uvt;
    Ray ray( p.data() );

    size_t tid = std::numeric_limits<size_t>::max();

    for( size_t i = 0; i < cs.size(); ++i )
    {
        tid = cs[i].value().payload();

        Point tc = cs[i].value().point();

        ASSERT( tid < nb_triags );

        idx[0] = triag_nodes(0,tid);
        idx[1] = triag_nodes(1,tid);
        idx[2] = triag_nodes(2,tid);

        ASSERT( idx[0] < inp_npts && idx[1] < inp_npts && idx[2] < inp_npts );

        Triag triag( icoords.slice(idx[0]), icoords.slice(idx[1]), icoords.slice(idx[2]) );

        found = triag_intersection( triag, ray, uvt );

#ifdef DUMP_PROJ
        if(found)
            of << "[SUCCESS]" << std::endl;
//        else
//            of << "[FAILED]" << std::endl;

        if(found)
        of << "   i    " << i << std::endl
           << "   ip   " << ip_ << std::endl
           << "   p    " << p << std::endl
           << "   tc   " << tc << std::endl
           << "   d    " << Point::distance(tc,p) << std::endl
           << "   tid  " << tid << std::endl
           << "   nidx " << idx[0] << " " << idx[1] << " " << idx[2] << std::endl
           << "   "
           << Point(icoords.slice(idx[0])) << " / "
           << Point(icoords.slice(idx[1])) << " / "
           << Point(icoords.slice(idx[2])) << std::endl
           << "   uvwt " << uvt << std::endl;
#endif
        if(found) // weights are the baricentric cooridnates u,v
        {
            phi[0] = uvt.w();
            phi[1] = uvt.u;
            phi[2] = uvt.v;
            break;
        }

    } // loop over nearest triangles

    return found;
}

//------------------------------------------------------------------------------------------------------

// static size_t factorial[12] = { 1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880, 3628800, 39916800 };
static size_t factorial[10] = { 1, 1, 2, 6, 24, 120, 720, 5040, 40320, 362880 };

void FiniteElement::compute( Grid& in, Grid& out, Eigen::SparseMatrix<double>& W ) const
{
    atlas::Mesh& i_mesh = in.mesh();
    atlas::Mesh& o_mesh = out.mesh();

    Timer t("compute weights");

    // generate mesh ...

    Tesselation::tesselate( in );

    // generate baricenters of each triangle & insert the baricenters on a kd-tree

    Tesselation::create_cell_centres( i_mesh );

    ptree.reset( create_cell_centre_index( i_mesh ) );

    // input mesh

    FunctionSpace&  i_nodes  = i_mesh.function_space( "nodes" );
    picoords = &i_nodes.field<double>( "coordinates" );

    FunctionSpace& triags = i_mesh.function_space( "triags" );

    ptriag_nodes = &triags.field<int>( "nodes" );

    nb_triags = triags.bounds()[1];
    inp_npts = i_nodes.bounds()[1];

    // output mesh

    FunctionSpace&  o_nodes  = o_mesh.function_space( "nodes" );
    FieldT<double>& ocoords  = o_nodes.field<double>( "coordinates" );

    const size_t out_npts = o_nodes.bounds()[1];

    // weights

    std::vector< Eigen::Triplet<double> > weights_triplets; /* structure to fill-in sparse matrix */

    weights_triplets.reserve( out_npts * 3 ); /* each row has 3 entries: one per vertice of triangle */

    /* search nearest k cell centres */

    boost::progress_display show_progress( out_npts );

    for( ip_ = 0; ip_ < out_npts; ++ip_ )
    {
        int idx[3]; /* indexes of the triangle that will contain the point*/
        Vector3d phi;
        Point p ( ocoords.slice(ip_) ); // lookup point

        size_t k = 1;
        while( ! project_point_to_triangle( p, phi, idx, factorial[k] ) )
        {
            ++k;
            if( k > (sizeof(factorial)/ sizeof(*factorial)) )
                throw eckit::TooManyRetries(k,"projecting point into tesselation");
        }

        ++show_progress;

        // insert the interpolant weights into the global (sparse) interpolant matrix

        for(int i = 0; i < 3; ++i)
            weights_triplets.push_back( Eigen::Triplet<double>( ip_, idx[i], phi[i] ) );
    }

    // fill-in sparse matrix

    W.setFromTriplets(weights_triplets.begin(), weights_triplets.end());
}

std::string FiniteElement::classname() const
{
    return "FiniteElement";
}

//------------------------------------------------------------------------------------------------------

} // namespace mir
