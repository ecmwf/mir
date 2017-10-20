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


#include "mir/method/fe/Conservative.h"

#include "eckit/linalg/LinearAlgebra.h"
#include "eckit/linalg/Vector.h"
#include "eckit/log/Log.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"
#include "atlas/array/IndexView.h"
#include "atlas/field/Field.h"
#include "atlas/interpolation/element/Quad3D.h"
#include "atlas/interpolation/element/Triag3D.h"
#include "atlas/mesh.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"


namespace mir {
namespace method {
namespace fe {


static const double oneThird = 1. / 3.;
static const double oneFourth = 1. / 4.;


Conservative::Conservative(const param::MIRParametrisation& param) :
    FELinear(param) {

    // output mesh requirements
    OutputMeshGenerationParams_.meshXYZField_ = true;
}


void Conservative::computeLumpedMassMatrix(eckit::linalg::Vector& d, const atlas::Mesh& mesh) const {
    using namespace atlas::mesh;
    eckit::Log::debug<LibMir>() << "Conservative::computeLumpedMassMatrix" << "\n" "Mesh " << mesh << std::endl;

    // FIXME this does not check for "virtual points"
    ASSERT(d.size() == mesh.nodes().size());
    d.setZero();

    const Nodes& nodes = mesh.nodes();
    const Cells& cells = mesh.cells();
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


void Conservative::assemble(util::MIRStatistics& statistics,
                            WeightMatrix& W,
                            const repres::Representation& in,
                            const repres::Representation& out ) const {
    eckit::Log::debug<LibMir>() << "Conservative::assemble (input: " << in << ", output: " << out << ")" << std::endl;


    // 1) IM_{ds} compute the interpolation matrix from destination (out) to source (input)
    WeightMatrix IM(in.numberOfPoints(), out.numberOfPoints());
    FELinear::assemble(statistics, IM, out, in);
    eckit::Log::debug<LibMir>() << "IM rows " << IM.rows() << " cols " << IM.cols() << std::endl;
    //    IM.save("IM.mat");


    // 2) M_s compute the lumped mass matrix (source mesh)
    util::MIRGrid gin(in.atlasGrid());
    eckit::linalg::Vector M_s(in.numberOfPoints());
    const atlas::Mesh& inputMesh = gin.mesh(statistics, InputMeshGenerationParams_);
    computeLumpedMassMatrix(M_s, inputMesh);


    // 3) M_d^{-1} compute the inverse lumped mass matrix (target mesh)
    util::MIRGrid gout(out.atlasGrid());
    eckit::linalg::Vector M_d(out.numberOfPoints());
    const atlas::Mesh& outputMesh = gout.mesh(statistics, OutputMeshGenerationParams_);
    computeLumpedMassMatrix(M_d, outputMesh);
    for (eckit::linalg::Scalar& v : M_d) {
        v = 1. / v;
    }


    // 4) W = M_d^{-1} . I^{T} . M_s
    W.reserve(IM.rows(), IM.cols(), IM.nonZeros()); // reserve same space as IM
    eckit::linalg::LinearAlgebra::backend().dsptd(M_d, IM, M_s, W);


    // 5) Normalise row weights
    WeightMatrix::iterator it(W);
    for (WeightMatrix::Size i = 0; i < W.rows(); ++i) {
        const WeightMatrix::iterator begin = W.begin(i);
        const WeightMatrix::iterator end   = W.end(i);

        // accumulate and re-distribute (linearly) the row weights
        double sum = 0.;
        for (it = begin; it != end; ++it) {
            sum += *it;
        }

        if (!eckit::types::is_approximately_equal(sum, 0.)) {
            const double factor =  1. / sum;
            for (it = begin; it != end; ++it) {
                *it *= factor;
            }
        }
    }
}


const char* Conservative::name() const {
    return "linear-conservative";
}


void Conservative::hash(eckit::MD5& md5) const {
    FELinear::hash(md5);
    md5.add(name());
}


void Conservative::print(std::ostream& out) const {
    out << "Conservative[]";
}


namespace {
static MethodBuilder<Conservative> __conservative("conservative");
}


}  // namespace fe
}  // namespace method
}  // namespace mir
