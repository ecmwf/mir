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

#include "eckit/types/Types.h"

#include "atlas/Tesselation.h"
#include "atlas/util/IndexView.h"

#include "atlas/PointIndex3.h"
#include "atlas/geometry/Ray.h"
#include "atlas/geometry/TriangleIntersection.h"
#include "atlas/geometry/QuadrilateralIntersection.h"
#include "atlas/meshgen/MeshGenerator.h"

#include "mir/param/MIRParametrisation.h"

using namespace eckit;

using atlas::Grid;
using atlas::Mesh;
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

namespace mir {
namespace method {

// void FiniteElement::Phi::print(std::ostream &s) const { s << "Phi[idx=" << idx << ",w=" << w << "]"; }


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


bool FiniteElement::project_point_to_element(Point &p, size_t done, size_t kpts ) const {

    IndexView<int, 2> triag_nodes ( *ptriag_nodes );
    IndexView<int, 2> quads_nodes ( *pquads_nodes );

    ArrayView<double, 2> icoords  ( *picoords     );

//    Log::info() << "kNearest done: " << done << " kpts: " << kpts << std::endl;

    ElemIndex3::NodeList cs = pTree_->kNearestNeighbours(p, kpts);

    // find in which element the point is contained
    // by computing the intercetion of the point with each nearest triangle

    Ray ray( p.data() );

    for ( size_t i = done; i < cs.size(); ++i ) {

        ElemPayload elem = cs[i].value().payload();

        if ( elem.type_ == 't') {
            phi_.resize(3);
            const size_t &tid = elem.id_;

            ASSERT( tid < nb_triags_ );

            phi_.idx[0] = triag_nodes(tid, 0);
            phi_.idx[1] = triag_nodes(tid, 1);
            phi_.idx[2] = triag_nodes(tid, 2);

            ASSERT( phi_.idx[0] < inp_npts_ && phi_.idx[1] < inp_npts_ && phi_.idx[2] < inp_npts_ );

            TriangleIntersection triag(icoords[phi_.idx[0]].data(),
                                       icoords[phi_.idx[1]].data(),
                                       icoords[phi_.idx[2]].data());

            Intersect is = triag.intersects(ray);

            //            Log::info() << is << std::endl;

            // weights are the baricentric cooridnates u,v
            if (is) {
                phi_.w[0] = 1. - is.u - is.v;
                phi_.w[1] = is.u;
                phi_.w[2] = is.v;
                //                Log::info() << p << " -> phi_ : " << phi_ << std::endl;
                return true;
            }
        } else {
            ASSERT(elem.type_ == 'q');

            phi_.resize(4);
            const size_t &qid = elem.id_;

            ASSERT( qid < nb_quads_ );

            phi_.idx[0] = quads_nodes(qid, 0);
            phi_.idx[1] = quads_nodes(qid, 1);
            phi_.idx[2] = quads_nodes(qid, 2);
            phi_.idx[3] = quads_nodes(qid, 3);

            ASSERT( phi_.idx[0] < inp_npts_ && phi_.idx[1] < inp_npts_ &&
                    phi_.idx[2] < inp_npts_ && phi_.idx[3] < inp_npts_ );

            QuadrilateralIntersection quad( icoords[phi_.idx[0]].data(),
                                            icoords[phi_.idx[1]].data(),
                                            icoords[phi_.idx[2]].data(),
                                            icoords[phi_.idx[3]].data() );

            Intersect is = quad.intersects(ray);

            //            Log::info() << is << std::endl;

            if (is) {
                phi_.w[0] = (1. - is.u) * (1. - is.v);
                phi_.w[1] =       is.u  * (1. - is.v);
                phi_.w[2] =       is.u  *       is.v ;
                phi_.w[3] = (1. - is.u) *       is.v ;
                //                Log::info() << p << " -> phi_ : " << phi_ << std::endl;
                return true;
            }
        }

    } // loop over nearest elements

    return false;
}


void FiniteElement::assemble(WeightMatrix &W, const Grid &in, const Grid &out) const {

    // FIXME arguments:
    eckit::Log::info() << "FiniteElement::assemble" << std::endl;

    Mesh &i_mesh = const_cast<Mesh &>(in.mesh());  // we modify the mesh when we tesselate
    Mesh &o_mesh = const_cast<Mesh &>(out.mesh());

    eckit::Timer timer("Compute weights");

    // generate mesh ...


    Tesselation::tesselate(in, i_mesh);

//    std::string meshGenerator;
//    ASSERT(parametrisation_.get("meshGenerator", meshGenerator));
//    eckit::ScopedPtr<MeshGenerator> meshGen( MeshGeneratorFactory::build(meshGenerator) );
//    meshGen->tesselate(in, i_mesh);

    // generate baricenters of each triangle & insert the baricenters on a kd-tree

    {
        eckit::Timer timer("Tesselation::create_cell_centres");
        Tesselation::create_cell_centres(i_mesh);
    }

    {
        eckit::Timer timer("Tcreate_element_centre_index");
        pTree_.reset(create_element_centre_index(i_mesh));
    }
    // input mesh

    FunctionSpace  &i_nodes  = i_mesh.function_space( "nodes" );
    picoords = &i_nodes.field<double>( "xyz" );

    FunctionSpace &triags = i_mesh.function_space( "triags" );
    ptriag_nodes = &triags.field<int>( "nodes" );

    FunctionSpace &quads = i_mesh.function_space( "quads" );
    pquads_nodes = &quads.field<int>( "nodes" );

    inp_npts_  = i_nodes.shape(0);
    nb_triags_ = triags.shape(0);
    nb_quads_  = quads.shape(0);

    Log::info() << "Mesh has " << eckit::Plural(nb_triags_, "triangle") << " and " << eckit::Plural(nb_quads_, "quadrilateral") << std::endl;

    // output mesh

    FunctionSpace  &o_nodes  = o_mesh.function_space( "nodes" );
    ArrayView<double, 2> ocoords ( o_nodes.field( "xyz" ) );

    out_npts_ = o_nodes.shape(0);

    // weights -- one per vertice of element, triangles (3) or quads (4)

    std::vector< Eigen::Triplet<double> > weights_triplets; /* structure to fill-in sparse matrix */

    weights_triplets.reserve( out_npts_ * 4 );

    /* search nearest k cell centres */

    size_t max_neighbours = 0;

    Log::info() << "Projecting " << eckit::Plural(out_npts_, "output point") << " to input mesh " << in.shortName() << std::endl;

    failed_.clear();

    for ( ip_ = 0; ip_ < out_npts_; ++ip_ ) {

        bool success = false;

        if (ip_ && (ip_ % 100000 == 0)) {
            double rate = ip_ / timer.elapsed();
            Log::info() << eckit::BigNum(ip_) << " ..."  << eckit::Seconds(timer.elapsed())
                        << ", rate: " << rate << " points/s, ETA: "
                        << eckit::ETA( (out_npts_ - ip_) / rate )
                        << std::endl;
        }

        Point p ( ocoords[ip_].data() ); // lookup point

        size_t done = 0;
        size_t kpts = 1;

        while (!(success = project_point_to_element(p, done, kpts))) {

            //            if(kpts>=1000)
            //                eckit::Log::info() << "Failed projecting to " << kpts << " elements ... " << std::endl;

            done = kpts;

            if (done >= nb_triags_ + nb_quads_) {
                failed_.push_back(p);
                Log::warning() << "Point " << ip_ << " with coords " << p << " failed projection ..." << std::endl;
                break;
            }

            kpts = std::min(4*done,nb_triags_+nb_quads_); // increase the number of searched elements
        }

        max_neighbours = std::max(done, max_neighbours);

        // insert the interpolant weights into the global (sparse) interpolant matrix
        if (success)
            for (int i = 0; i < phi_.size(); ++i)
                weights_triplets.push_back( Eigen::Triplet<double>( ip_, phi_.idx[i], phi_.w[i] ) );
    }

    Log::info() << "Projected " << ip_ - failed_.size() << " points"  << std::endl;
    Log::info() << "Maximum neighbours searched " << max_neighbours << " elements"  << std::endl;

    if (failed_.size()) {
        std::ostringstream os;
        os << "Failed to project following points into input Grid " << in.shortName() << ":" << std::endl;
        for (size_t i = 0; i < failed_.size(); ++i)
            os << failed_[i] << std::endl;
        throw SeriousBug(os.str(), Here());
    }

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

