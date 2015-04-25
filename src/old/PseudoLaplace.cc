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

#include "eckit/maths/Eigen.h"
#include "eckit/config/Resource.h"
#include "eckit/log/Log.h"
#include "eckit/utils/Translator.h"

#include "atlas/util/ArrayView.h"
#include "atlas/Parameters.h"

#include "mir/PseudoLaplace.h"

//------------------------------------------------------------------------------------------------------

using namespace Eigen;
using namespace atlas;

namespace mir {

//------------------------------------------------------------------------------------------------------

PseudoLaplace::PseudoLaplace() : KNearest()
{
}

PseudoLaplace::~PseudoLaplace()
{
}

void PseudoLaplace::compute( Grid& in, Grid& out, Weights::Matrix& W ) const
{
    build_sptree(in);

    atlas::Mesh& o_mesh = out.mesh();

    // output points
    FunctionSpace&  o_nodes  = o_mesh.function_space( "nodes" );
	ArrayView<double,2> ocoords ( o_nodes.field( "xyz" ) );

    const size_t out_npts = o_nodes.shape(0);

    // init structure used to fill in sparse matrix
    std::vector< Eigen::Triplet<double> > weights_triplets; 
    weights_triplets.reserve( out_npts * nclosest_ );

    std::vector<atlas::PointIndex3::Value> closest;
    
    VectorXd Dx(nclosest_);
    VectorXd Dy(nclosest_);
    VectorXd Dz(nclosest_);

    std::vector<double> weights;
    weights.reserve(nclosest_);

    for( size_t ip = 0; ip < out_npts; ++ip)
    {
        // get the reference output point
        eckit::geometry::Point3 p ( ocoords[ip].data() );
        
        // find the closest input points to this output
        sptree_->closestNPoints(p, nclosest_, closest);

        const size_t npts = closest.size();

        // then calculate the nearest neighbour weights
        weights.resize(npts, 0.0);

        double Ixx(0),Ixy(0),Ixz(0),Iyy(0),Iyz(0),Izz(0), Rx(0),Ry(0),Rz(0), Lx,Ly,Lz, dx,dy,dz;

        for( size_t j = 0; j < npts; ++j)
        {
            eckit::geometry::Point3 np  = closest[j].point();

            dx = np[XX] - p[XX];
            dy = np[YY] - p[YY];
            dz = np[ZZ] - p[ZZ];

            Ixx += dx*dx;
            Ixy += dx*dy;
            Ixz += dx*dz;
            Iyy += dy*dy;
            Iyz += dy*dz;
            Izz += dz*dz;

            Rx += dx;
            Ry += dy;
            Rz += dz;

            Dx[j]=dx;
            Dy[j]=dy;
            Dz[j]=dz;
        }

        Lx =  (-(Iyz*Iyz*Rx) + Iyy*Izz*Rx + Ixz*Iyz*Ry - Ixy*Izz*Ry - Ixz*Iyy*Rz + Ixy*Iyz*Rz)/
                (Ixz*Ixz*Iyy - 2.*Ixy*Ixz*Iyz + Ixy*Ixy*Izz + Ixx*(Iyz*Iyz - Iyy*Izz));
        Ly =  (Ixz*Iyz*Rx - Ixy*Izz*Rx - Ixz*Ixz*Ry + Ixx*Izz*Ry + Ixy*Ixz*Rz - Ixx*Iyz*Rz)/
                (Ixz*Ixz*Iyy - 2.*Ixy*Ixz*Iyz + Ixx*Iyz*Iyz + Ixy*Ixy*Izz - Ixx*Iyy*Izz);
        Lz =  (-(Ixz*Iyy*Rx) + Ixy*Iyz*Rx + Ixy*Ixz*Ry - Ixx*Iyz*Ry - Ixy*Ixy*Rz + Ixx*Iyy*Rz)/
                (Ixz*Ixz*Iyy - 2.*Ixy*Ixz*Iyz + Ixy*Ixy*Izz + Ixx*(Iyz*Iyz - Iyy*Izz));

        double S = 0;
        for( size_t j = 0; j < npts; ++j )
        {
            weights[j] = 1.0 + Lx*Dx[j] + Ly*Dy[j] + Lz*Dz[j];
            S += weights[j];
        }

        for( size_t j = 0; j < npts; ++j )
            weights[j] /= S;

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

std::string PseudoLaplace::classname() const
{
    std::string ret ("PseudoLaplace");
    ret += eckit::Translator<size_t,std::string>()(nclosest_);
    return ret;
}

//------------------------------------------------------------------------------------------------------

} // namespace mir
