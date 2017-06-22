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


#include "mir/method/Conservative.h"

#include "eckit/log/Log.h"
#include "eckit/linalg/Vector.h"
#include "eckit/linalg/LinearAlgebra.h"
#include "atlas/grid.h"
#include "atlas/field/Field.h"
#include "atlas/mesh/Mesh.h"
#include "atlas/mesh/Nodes.h"
#include "atlas/mesh/Elements.h"
#include "atlas/mesh/ElementType.h"
#include "atlas/array/IndexView.h"
#include "atlas/mesh/actions/BuildXYZField.h"
#include "atlas/interpolation/element/Triag3D.h"
#include "atlas/interpolation/element/Quad3D.h"
#include "mir/config/LibMir.h"
#include "mir/util/MIRGrid.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"

using eckit::linalg::Vector;
using eckit::linalg::LinearAlgebra;
using atlas::Mesh;
using atlas::interpolation::element::Triag3D;
using atlas::interpolation::element::Quad3D;


namespace mir {
namespace method {


static const double oneThird  = 1./ 3.;
static const double oneFourth = 1./ 4.;

Conservative::Conservative(const param::MIRParametrisation &param) :
    FELinear(param) {
}

Conservative::~Conservative() {
}

void Conservative::computeLumpedMassMatrix(eckit::linalg::Vector& d, const repres::Representation& representation, atlas::Mesh& mesh) const
{

    const util::MIRGrid& grid = representation.grid();

    eckit::Log::debug<LibMir>() << "Conservative::computeLumpedMassMatrix" << std::endl;

    eckit::Log::debug<LibMir>() << "Mesh " << mesh << std::endl;

    d.resize(grid.size());

    d.setZero();

    atlas::mesh::actions::BuildXYZField("xyz")(mesh); // ensure we have a 'xyz' field (output mesh may not have it)

    const atlas::mesh::Nodes& nodes  = mesh.nodes();
    atlas::array::ArrayView<double, 2> coords = atlas::array::make_view< double, 2 >(nodes.field( "xyz" ));

// TODO we need to consider points that are virtual
//    size_t firstVirtualPoint = std::numeric_limits<size_t>::max();
//    if( nodes.metadata().has("NbRealPts") )
//        firstVirtualPoint = i_nodes.metadata().get<size_t>("NbRealPts");

/// TODO Must handle missing values

    const atlas::mesh::Cells& cells = mesh.cells();
    for( size_t jtype=0; jtype<cells.nb_types(); ++jtype )
    {
      const atlas::mesh::Elements& elements = cells.elements(jtype);
      const atlas::mesh::ElementType& element_type = elements.element_type();
      const atlas::mesh::BlockConnectivity& node_connectivity = elements.node_connectivity();

      if( element_type.name() == "Triangle" )
      {
        const size_t nb_triags = elements.size();
        for(size_t e = 0; e < nb_triags; ++e)
        {
            size_t idx [3];
            for(size_t n = 0; n<3; ++n)
              idx[n] = node_connectivity(e,n);

            Triag3D triag(coords[idx[0]].data(), coords[idx[1]].data(), coords[idx[2]].data());

            const double area = triag.area();

            /// TODO add check for virtuals

            for(size_t n = 0; n<3; ++n)
                d[ idx[n] ] += area * oneThird;
        }
      }

      if( element_type.name() == "Quadrilateral" )
      {
        const size_t nb_quads = elements.size();
        for(size_t e = 0; e < nb_quads; ++e)
        {
            size_t idx [4];
            for(size_t n = 0; n<4; ++n)
              idx[n] = node_connectivity(e,n);

            Quad3D quad(coords[idx[0]].data(), coords[idx[1]].data(), coords[idx[2]].data(), coords[idx[3]].data());

            const double area = quad.area();

            /// TODO add check for virtuals

            for(size_t n = 0; n<4; ++n)
                d[ idx[n] ] += area * oneFourth;
        }
      }

    }
}

void Conservative::assemble(WeightMatrix& W, const repres::Representation& rin, const repres::Representation& rout) const {

    util::MIRGrid in(rin.grid());
    util::MIRGrid out(rout.grid());

    eckit::Log::debug<LibMir>() << "Conservative::assemble (input: " << rin << ", output: " << rout << ")" << std::endl;

    // 1) IM_{ds} compute the interpolation matrix from destination (out) to source (input)

    WeightMatrix IM(in.size(), out.size());

    FELinear::assemble(IM, rout, rin);

    eckit::Log::debug<LibMir>()
            << "IM rows " << IM.rows()
            << " cols "   << IM.cols() << std::endl;

//    IM.save("IM.mat");

    // 2) M_s compute the lumped mass matrix of the source mesh

    Vector M_s;
    computeLumpedMassMatrix(M_s, rin, in.mesh());

    // 3) M_d^{-1} compute the inverse lumped mass matrix of the destination mesh

    Vector M_d;
    computeLumpedMassMatrix(M_d, rout, out.mesh());

    for(size_t i = 0; i < M_d.size(); ++i)
        M_d[i] = 1./M_d[i];

    // 4) W = M_d^{-1} . I^{T} . M_s

    W.reserve(IM.rows(), IM.cols(), IM.nonZeros()); // reserve same space as IM

    LinearAlgebra::backend().dsptd(M_d, IM, M_s, W);
}

const char* Conservative::name() const {
    return  "conservative";
}

void Conservative::hash(eckit::MD5 &md5) const {
    FELinear::hash(md5);
}

void Conservative::print(std::ostream &out) const {
    out << "Conservative[]";
}

namespace {
static MethodBuilder< Conservative > __conservative("conservative");
}


}  // namespace method
}  // namespace mir

