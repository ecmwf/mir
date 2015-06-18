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

#include "eckit/la/Vector.h"

#include "mir/method/PseudoLaplace.h"

#include <string>

#include "mir/util/PointSearch.h"
#include "mir/param/MIRParametrisation.h"
#include "atlas/actions/BuildXYZField.h"


namespace mir {
namespace method {


PseudoLaplace::PseudoLaplace(const param::MIRParametrisation& param) :
    MethodWeighted(param),
    nclosest_(4) {

    param.get("nclosest", nclosest_);

}

PseudoLaplace::~PseudoLaplace() {
}

const char* PseudoLaplace::name() const {
    return  "pseudo-laplace";
}

void PseudoLaplace::hash( eckit::MD5& md5) const {
    MethodWeighted::hash(md5);
    md5 << nclosest_;
}

void PseudoLaplace::assemble(WeightMatrix& W, const atlas::Grid& in, const atlas::Grid& out) const {

    util::PointSearch  sptree(in.mesh());

    const atlas::Mesh& o_mesh = out.mesh();

    // output points
    atlas::FunctionSpace& o_nodes  = o_mesh.function_space( "nodes" );
    atlas::actions::BuildXYZField("xyz")(o_nodes);
    atlas::ArrayView<double,2> ocoords ( o_nodes.field( "xyz" ) );

    const size_t out_npts = o_nodes.shape(0);

    // init structure used to fill in sparse matrix
    std::vector< WeightMatrix::Triplet > weights_triplets;
    weights_triplets.reserve( out_npts * nclosest_ );

    std::vector<atlas::PointIndex3::Value> closest;

    eckit::la::Vector Dx(nclosest_);
    eckit::la::Vector Dy(nclosest_);
    eckit::la::Vector Dz(nclosest_);

    std::vector<double> weights;
    weights.reserve(nclosest_);

    for( size_t ip = 0; ip < out_npts; ++ip) {
        // get the reference output point
        eckit::geometry::Point3 p ( ocoords[ip].data() );

        // find the closest input points to this output
        sptree.closestNPoints(p, nclosest_, closest);

        const size_t npts = closest.size();

        // then calculate the nearest neighbour weights
        weights.resize(npts, 0.0);

        double Ixx(0),Ixy(0),Ixz(0),Iyy(0),Iyz(0),Izz(0), Rx(0),Ry(0),Rz(0), Lx,Ly,Lz, dx,dy,dz;

        for( size_t j = 0; j < npts; ++j) {
            eckit::geometry::Point3 np  = closest[j].point();
            using atlas::XX;
            using atlas::YY;
            using atlas::ZZ;

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

            Dx[j] = dx;
            Dy[j] = dy;
            Dz[j] = dz;
        }

        Lx =  (-(Iyz*Iyz*Rx) + Iyy*Izz*Rx + Ixz*Iyz*Ry - Ixy*Izz*Ry - Ixz*Iyy*Rz + Ixy*Iyz*Rz)/
              (Ixz*Ixz*Iyy - 2.*Ixy*Ixz*Iyz + Ixy*Ixy*Izz + Ixx*(Iyz*Iyz - Iyy*Izz));
        Ly =  (Ixz*Iyz*Rx - Ixy*Izz*Rx - Ixz*Ixz*Ry + Ixx*Izz*Ry + Ixy*Ixz*Rz - Ixx*Iyz*Rz)/
              (Ixz*Ixz*Iyy - 2.*Ixy*Ixz*Iyz + Ixx*Iyz*Iyz + Ixy*Ixy*Izz - Ixx*Iyy*Izz);
        Lz =  (-(Ixz*Iyy*Rx) + Ixy*Iyz*Rx + Ixy*Ixz*Ry - Ixx*Iyz*Ry - Ixy*Ixy*Rz + Ixx*Iyy*Rz)/
              (Ixz*Ixz*Iyy - 2.*Ixy*Ixz*Iyz + Ixy*Ixy*Izz + Ixx*(Iyz*Iyz - Iyy*Izz));

        double S = 0;
        for( size_t j = 0; j < npts; ++j ) {
            weights[j] = 1.0 + Lx*Dx[j] + Ly*Dy[j] + Lz*Dz[j];
            S += weights[j];
        }

        for( size_t j = 0; j < npts; ++j )
            weights[j] /= S;

        // insert the interpolant weights into the global (sparse) interpolant matrix
        for(size_t i = 0; i < npts; ++i) {
            size_t index = closest[i].payload();
            weights_triplets.push_back( WeightMatrix::Triplet( ip, index, weights[i] ) );
        }
    }

    // fill-in sparse matrix
    W.setFromTriplets(weights_triplets);
}


void PseudoLaplace::print(std::ostream&) const {
}


namespace {
static MethodBuilder< PseudoLaplace > __pseudolaplace("pseudo-laplace");
}


}  // namespace method
}  // namespace mir

