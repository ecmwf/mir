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

#include "eckit/config/Resource.h"
#include "eckit/log/Log.h"
#include "eckit/utils/Translator.h"

#include "atlas/util/ArrayView.h"

#include "mir/KNearest.h"

//------------------------------------------------------------------------------------------------------

using eckit::Resource;

using atlas::Grid;
using atlas::FunctionSpace;
using atlas::ArrayView;

namespace mir {

//------------------------------------------------------------------------------------------------------

KNearest::KNearest() 
{
    nclosest_ = Resource<unsigned>( "KNearest;$MIR_KNEAREST", 4 );
    if( nclosest_ == 0 )
        throw eckit::UserError( "KNearest k closest points cannot be 0", Here() );
}

KNearest::KNearest(const size_t& k) : nclosest_(k)
{
}

KNearest::KNearest(const size_t &k, Grid &in) : nclosest_(k)
{

}

KNearest::~KNearest() 
{
}

void KNearest::compute( Grid& in, Grid& out, Weights::Matrix& W ) const
{
    build_sptree(in);

    atlas::Mesh& o_mesh = out.mesh();

    // output points
    FunctionSpace&  o_nodes  = o_mesh.function_space( "nodes" );
    ArrayView<double,2> ocoords ( o_nodes.field( "coordinates" ) );

    const size_t out_npts = o_nodes.shape(0);

    // init structure used to fill in sparse matrix
    std::vector< Eigen::Triplet<double> > weights_triplets; 
    weights_triplets.reserve( out_npts * nclosest_ );

    std::vector<atlas::PointIndex3::Value> closest;

    std::vector<double> weights;
    weights.reserve(nclosest_);

    /// @todo take epsilon from some general config
    const double epsilon = Resource<double>( "KNearestEpsilon", std::numeric_limits<double>::epsilon() );

    for( size_t ip = 0; ip < out_npts; ++ip)
    {
        // get the reference output point
        eckit::geometry::Point3 p ( ocoords[ip].data() );
        
        // find the closest input points to this output
        sptree_->closestNPoints(p, nclosest_, closest);
        
        const size_t npts = closest.size();

        // then calculate the nearest neighbour weights
        weights.resize(npts, 0.0);

        // sum all calculated weights for normalisation
        double sum = 0.0;

        for( size_t j = 0; j < npts; ++j )
        {
            // one of the closest points
            eckit::geometry::Point3 np  = closest[j].point();

            // calculate distance squared and weight
            const double d2 = eckit::geometry::Point3::distance2(p, np);
            weights[j] = 1.0 / ( epsilon + d2 );

            // also work out the total
            sum += weights[j];
        }

        ASSERT( sum > 0.0 );

        // now normalise all weights according to the total
        for( size_t j = 0; j < npts; j++)
        {
            weights[j] /= sum;
        }

        // insert the interpolant weights into the global (sparse) interpolant matrix
        for(int i = 0; i < npts; ++i)
        {
            size_t index = closest[i].payload();
            weights_triplets.push_back( Eigen::Triplet<double>( ip, index, weights[i] ) );
        }
    }

    // fill-in sparse matrix
    W.setFromTriplets(weights_triplets.begin(), weights_triplets.end());
    
}

std::string KNearest::classname() const
{
    std::string ret ("KNearest");
    ret += eckit::Translator<size_t,std::string>()(nclosest_);
    return ret;
}

void KNearest::build_sptree( Grid& in ) const
{
    atlas::Mesh& i_mesh = in.mesh();

    std::string inhash = in.hash();
    if( inhash != hash_ )
        sptree_.reset( new PointSearch(i_mesh) );

    hash_ = inhash;
}

//------------------------------------------------------------------------------------------------------

} // namespace mir
