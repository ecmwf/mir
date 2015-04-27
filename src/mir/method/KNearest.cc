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
/// @date Apr 2015


#include <string>

#include "eckit/config/Resource.h"
#include "mir/util/PointSearch.h"

#include "mir/method/KNearest.h"


namespace mir {
namespace method {


KNearest::KNearest(const param::MIRParametrisation& param) :
    MethodWeighted(param,"method.k-nearest"),
    nclosest_(4),
    epsilon_(eckit::Resource<double>( "KNearestEpsilon", std::numeric_limits<double>::epsilon() )) {
    // FIXME  all these construction-time parameters (should come from param)
}


KNearest::~KNearest() {
}


void KNearest::assemble(MethodWeighted::Matrix& W, const atlas::Grid& in, const atlas::Grid& out) const {

    sptree_->build_sptree(in);

    const atlas::Mesh& o_mesh = out.mesh();

    // output points
    atlas::FunctionSpace& o_nodes  = o_mesh.function_space( "nodes" );
    atlas::ArrayView<double,2> ocoords ( o_nodes.field( "xyz" ) );

    const size_t out_npts = o_nodes.shape(0);

    // init structure used to fill in sparse matrix
    std::vector< Eigen::Triplet<double> > weights_triplets;
    weights_triplets.reserve( out_npts * nclosest_ );

    std::vector<atlas::PointIndex3::Value> closest;

    std::vector<double> weights;
    weights.reserve(nclosest_);

    for( size_t ip = 0; ip < out_npts; ++ip) {
        // get the reference output point
        eckit::geometry::Point3 p ( ocoords[ip].data() );

        // find the closest input points to this output
        sptree_->closestNPoints(p, nclosest_, closest);

        const size_t npts = closest.size();

        // then calculate the nearest neighbour weights
        weights.resize(npts, 0.0);

        // sum all calculated weights for normalisation
        double sum = 0.0;

        for( size_t j = 0; j < npts; ++j ) {
            // one of the closest points
            eckit::geometry::Point3 np  = closest[j].point();

            // calculate distance squared and weight
            const double d2 = eckit::geometry::Point3::distance2(p, np);
            weights[j] = 1.0 / ( epsilon_ + d2 );

            // also work out the total
            sum += weights[j];
        }

        ASSERT( sum > 0.0 );

        // now normalise all weights according to the total
        for( size_t j = 0; j < npts; j++) {
            weights[j] /= sum;
        }

        // insert the interpolant weights into the global (sparse) interpolant matrix
        for(int i = 0; i < npts; ++i) {
            size_t index = closest[i].payload();
            weights_triplets.push_back( Eigen::Triplet<double>( ip, index, weights[i] ) );
        }
    }

    // fill-in sparse matrix
    W.setFromTriplets(weights_triplets.begin(), weights_triplets.end());
}


void KNearest::print(std::ostream&) const {
}


namespace {
static MethodBuilder< KNearest > __knearest("method.k-nearest");
}


}  // namespace method
}  // namespace mir

