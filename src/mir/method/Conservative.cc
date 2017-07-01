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
/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date May 2015


#include "mir/method/Conservative.h"

#include "eckit/linalg/LinearAlgebra.h"
#include "eckit/linalg/Vector.h"
#include "eckit/log/Log.h"
#include "atlas/array/IndexView.h"
#include "atlas/field/Field.h"
#include "atlas/grid.h"
#include "atlas/interpolation/element/Quad3D.h"
#include "atlas/interpolation/element/Triag3D.h"
#include "atlas/mesh.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"



namespace mir {
namespace method {


static const double oneThird = 1. / 3.;
static const double oneFourth = 1. / 4.;


Conservative::Conservative(const param::MIRParametrisation& param) : FELinear(param) {
}


void Conservative::computeLumpedMassMatrix(eckit::linalg::Vector& d, const util::MIRGrid& grid) const {
    using namespace atlas::mesh;

    eckit::Log::debug<LibMir>() << "Conservative::computeLumpedMassMatrix"
                                << "\n" "Mesh " << grid.mesh()
                                << std::endl;

    d.resize(grid.size());
    d.setZero();

    const Nodes& nodes = grid.mesh().nodes();
    const Cells& cells = grid.mesh().cells();
    atlas::array::ArrayView<double, 2> coords = atlas::array::make_view<double, 2>(nodes.field("xyz"));

    // TODO handle missing values
    // TODO we need to consider points that are virtual
    //    size_t firstVirtualPoint = std::numeric_limits<size_t>::max();
    //    if( nodes.metadata().has("NbRealPts") )
    //        firstVirtualPoint = i_nodes.metadata().get<size_t>("NbRealPts");


    for (size_t jtype = 0; jtype < cells.nb_types(); ++jtype) {
        const Elements& elements = cells.elements(jtype);
        const BlockConnectivity& connectivity = elements.node_connectivity();

        const std::string& type = elements.element_type().name();
        size_t idx[4];
        d.setZero();

        if (type == "Triangle") {
            for (size_t e = 0; e < elements.size(); ++e) {
                for (size_t n = 0; n < 3; ++n) {
                    idx[n] = size_t(connectivity(e, n));
                }

                atlas::interpolation::element::Triag3D triag(coords[idx[0]].data(), coords[idx[1]].data(), coords[idx[2]].data());

                const double nodalDistribution = triag.area() * oneThird;
                for (size_t n = 0; n < 3; ++n) {
                    d[idx[n]] += nodalDistribution;
                }
            }
        } else if (type == "Quadrilateral") {
            for (size_t e = 0; e < elements.size(); ++e) {
                for (size_t n = 0; n < 4; ++n) {
                    idx[n] = size_t(connectivity(e, n));
                }

                atlas::interpolation::element::Quad3D quad(coords[idx[0]].data(), coords[idx[1]].data(), coords[idx[2]].data(), coords[idx[3]].data());

                const double nodalDistribution = quad.area() * oneFourth;
                for (size_t n = 0; n < 4; ++n) {
                    d[idx[n]] += nodalDistribution;
                }
            }
        } else {
            throw eckit::SeriousBug("Found unsupported element in mesh ('" + type + "'), "
                                    "only 'Triangle' and 'Quadrilateral' are supported",
                                    Here());
        }
    }
}


void Conservative::assemble(
        util::MIRStatistics& statistics,
        WeightMatrix& W,
        const repres::Representation& rin,
        const repres::Representation& rout ) const {
    util::MIRGrid in(rin.grid(statistics, meshgenparams_));
    util::MIRGrid out(rout.grid(statistics, meshgenparams_));

    eckit::Log::debug<LibMir>() << "Conservative::assemble (input: " << rin << ", output: " << rout << ")" << std::endl;

    // 1) IM_{ds} compute the interpolation matrix from destination (out) to source (input)
    WeightMatrix IM(in.size(), out.size());
    FELinear::assemble(statistics, IM, rout, rin);
    eckit::Log::debug<LibMir>() << "IM rows " << IM.rows() << " cols " << IM.cols() << std::endl;
    //    IM.save("IM.mat");

    // 2) M_s compute the lumped mass matrix (source mesh)
    eckit::linalg::Vector M_s;
    computeLumpedMassMatrix(M_s, in);

    // 3) M_d^{-1} compute the inverse lumped mass matrix (target mesh)
    eckit::linalg::Vector M_d;
    computeLumpedMassMatrix(M_d, out);
    for (eckit::linalg::Scalar& v : M_d) {
        v = 1. / v;
    }

    // 4) W = M_d^{-1} . I^{T} . M_s
    W.reserve(IM.rows(), IM.cols(), IM.nonZeros()); // reserve same space as IM
    eckit::linalg::LinearAlgebra::backend().dsptd(M_d, IM, M_s, W);
}


const char* Conservative::name() const {
    return "conservative";
}


void Conservative::hash(eckit::MD5& md5) const {
    FELinear::hash(md5);
}


void Conservative::print(std::ostream& out) const {
    out << "Conservative[]";
}


namespace {
static MethodBuilder<Conservative> __conservative("conservative");
}


} // namespace method
} // namespace mir
