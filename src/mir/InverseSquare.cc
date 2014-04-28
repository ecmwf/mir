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
#include "InverseSquare.h"
#include "eckit/log/Log.h"
#include "PointSearch.h"
#include <boost/progress.hpp>


//-----------------------------------------------------------------------------
using Eigen::SparseMatrix;
using atlas::FunctionSpace;
using atlas::FieldT;

namespace mir {

InverseSquare::InverseSquare() 
{
    eckit::Log::info() << "Build a InverseSquare" << std::endl;
}

InverseSquare::~InverseSquare() 
{
    eckit::Log::info() << "Destroy a InverseSquare" << std::endl;
}

void InverseSquare::compute( atlas::Mesh& i_mesh, atlas::Mesh& o_mesh, Eigen::SparseMatrix<double>& W ) const
{

    // input points
    FunctionSpace&  i_nodes  = i_mesh.function_space( "nodes" );
    FieldT<double>& icoords = i_nodes.field<double>( "coordinates" );
    

    // output points
    FunctionSpace&  o_nodes  = o_mesh.function_space( "nodes" );
    FieldT<double>& ocoords  = o_nodes.field<double>( "coordinates" );

    const size_t out_npts = o_nodes.bounds()[1];

    // Number of closest points to consider
    const size_t nclosest = 4;

    // init structure used to fill in sparse matrix
    std::vector< Eigen::Triplet<double> > weights_triplets; 
    weights_triplets.reserve( out_npts * nclosest ); 

    // initialise progress bar
    boost::progress_display show_progress( out_npts );

    // this baby forms a kd-tree of the input mesh
    PointSearch ps(i_mesh);

    std::vector<atlas::PointIndex3::Value> closest;
    
    /// @todo take epsilon from some general config
    const double epsilon = 1e-08;

    for( size_t ip = 0; ip < out_npts; ++ip)
    {
        // get the reference output point
        eckit::geometry::Point3 p ( ocoords.slice(ip) ); 
        
        // find the closest input points to this output
        ps.closestNPoints(p, nclosest, closest);
        
        // then calculate the nearest neighbour weights
        std::vector<double> weights;
        weights.resize(closest.size(), 0.0);

        // sum all calculated weights for normalisation
        double sum = 0.0;

        for( size_t j = 0; j < closest.size(); j++)
        {
            // one of the closest points
            eckit::geometry::Point3 np  = closest[j].point();

            // calculate distance squared and weight
            const double d2 = eckit::geometry::Point3::distance2(p, np);
            weights[j] = 1.0 / ( epsilon + d2 );

            // also work out the total
            sum += weights[j];
        }

        // now normalise all weights according to the total
        for( size_t j = 0; j < weights.size(); j++)
        {
            ASSERT(sum != 0.0);
            weights[j] /= sum;
        }

        ++show_progress;

        // insert the interpolant weights into the global (sparse) interpolant matrix
        for(int i = 0; i < closest.size(); ++i)
        {
            size_t index = closest[i].payload();
            weights_triplets.push_back( Eigen::Triplet<double>( ip, index, weights[i] ) );
        }
    }

    // fill-in sparse matrix
    W.setFromTriplets(weights_triplets.begin(), weights_triplets.end());
    
}
} // namespace mir
