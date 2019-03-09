/*
 * (C) Copyright 1996- ECMWF.
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

#include <algorithm>

#include "eckit/linalg/LinearAlgebra.h"
#include "eckit/linalg/Vector.h"
#include "eckit/log/Log.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "atlas/interpolation/element/Quad3D.h"
#include "atlas/interpolation/element/Triag3D.h"

#include "mir/config/LibMir.h"
#include "mir/repres/Representation.h"
#include "mir/util/MIRGrid.h"


namespace mir {
namespace method {
namespace fe {


Conservative::Conservative(const param::MIRParametrisation& param) :
    FELinear(param),
    outputMeshGenerationParams_("output", param) {
}


bool Conservative::sameAs(const Method& other) const {
    auto o = dynamic_cast<const Conservative*>(&other);
    return o
            && outputMeshGenerationParams_.sameAs(o->outputMeshGenerationParams_)
            && FELinear::sameAs(other);
}


void Conservative::computeLumpedMassMatrix(eckit::linalg::Vector& d, const atlas::Mesh& mesh) const {
    eckit::Log::debug<LibMir>() << "Conservative::computeLumpedMassMatrix" << "\n" "Mesh " << mesh << std::endl;

    const auto& nodes = mesh.nodes();
    const auto coords = atlas::array::make_view<double, 2, atlas::array::Intent::ReadOnly>(nodes.field("xyz"));

    const auto nbRealPts = nodes.metadata().has("NbRealPts") ? nodes.metadata().get<size_t>("NbRealPts")
                                                             : size_t(nodes.size());
    ASSERT(0 < d.size() && d.size() == nbRealPts);
    d.setZero();

    for (auto jtype = 0; jtype < mesh.cells().nb_types(); ++jtype) {
        const auto& connectivity = mesh.cells().elements(jtype).node_connectivity();

        /* assumes:
         * - nb_cols == 3 implies triangle
         * - nb_cols == 4 implies quadrilateral
         * - no other element is supported at the time
         */
        const auto nb_cols = size_t(connectivity.cols());
        ASSERT(nb_cols == 3 || nb_cols == 4);

        for (auto e = 0; e < connectivity.rows(); ++e) {

            std::vector<size_t> idx(nb_cols);
            for (size_t n = 0; n < nb_cols; ++n) {
                idx[n] = size_t(connectivity(e, atlas::idx_t(n)));
            }

            if (std::any_of(idx.begin(), idx.end(), [=](size_t n) { return n >= nbRealPts; })) {
                continue;  // element has a virtual point
            }

            static const double oneThird = 1. / 3.;
            static const double oneFourth = 1. / 4.;

            const double nodalDistribution =
                    nb_cols == 3 ?
                        oneThird * atlas::interpolation::element::Triag3D(
                            atlas::PointXYZ{ coords(idx[0], 0), coords(idx[0], 1), coords(idx[0], 2) },
                            atlas::PointXYZ{ coords(idx[1], 0), coords(idx[1], 1), coords(idx[1], 2) },
                            atlas::PointXYZ{ coords(idx[2], 0), coords(idx[2], 1), coords(idx[2], 2) }).area()
                      : oneFourth * atlas::interpolation::element::Quad3D(
                            atlas::PointXYZ{ coords(idx[0], 0), coords(idx[0], 1), coords(idx[0], 2) },
                            atlas::PointXYZ{ coords(idx[1], 0), coords(idx[1], 1), coords(idx[1], 2) },
                            atlas::PointXYZ{ coords(idx[2], 0), coords(idx[2], 1), coords(idx[2], 2) },
                            atlas::PointXYZ{ coords(idx[3], 0), coords(idx[3], 1), coords(idx[3], 2) }).area();
            ASSERT(nodalDistribution > 0.);

            for (auto n : idx) {
                d[n] += nodalDistribution;
            }
        }
    }
}


void Conservative::assemble(util::MIRStatistics& statistics,
                            WeightMatrix& W,
                            const repres::Representation& in,
                            const repres::Representation& out ) const {
    eckit::Log::debug<LibMir>() << "Conservative::assemble (input: " << in << ", output: " << out << ")" << std::endl;

    // 0) let representations set the mesh generator parameters
    auto inputMeshGenerationParams = inputMeshGenerationParams_;
    in.fill(inputMeshGenerationParams);

    auto outputMeshGenerationParams = outputMeshGenerationParams_;
    out.fill(outputMeshGenerationParams);


    // 1) IM_{ds} compute the interpolation matrix from destination (out) to source (input)
    WeightMatrix IM(out.numberOfPoints(), in.numberOfPoints());
    FELinear::assemble(statistics, IM, in, out);
    eckit::Log::debug<LibMir>() << "IM rows " << IM.rows() << " cols " << IM.cols() << std::endl;
    //    IM.save("IM.mat");


    // 2) M_s compute the lumped mass matrix (source mesh)
    util::MIRGrid gin(in.atlasGrid());
    eckit::linalg::Vector M_s(in.numberOfPoints());
    const atlas::Mesh& inputMesh = gin.mesh(statistics, inputMeshGenerationParams);
    computeLumpedMassMatrix(M_s, inputMesh);


    // 3) M_d^{-1} compute the inverse lumped mass matrix (target mesh)
    util::MIRGrid gout(out.atlasGrid());
    eckit::linalg::Vector M_d(out.numberOfPoints());
    const atlas::Mesh& outputMesh = gout.mesh(statistics, outputMeshGenerationParams);
    computeLumpedMassMatrix(M_d, outputMesh);
    for (eckit::linalg::Scalar& v : M_d) {
        v = eckit::types::is_approximately_equal(v, 0.) ? 0. : 1. / v;
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
    outputMeshGenerationParams_.hash(md5);
    md5.add(name());
}


void Conservative::print(std::ostream& out) const {
    out << "Conservative[";
    FELinear::print(out);
    out << "]";
}


namespace {
static MethodBuilder<Conservative> __conservative("conservative");
}


}  // namespace fe
}  // namespace method
}  // namespace mir
