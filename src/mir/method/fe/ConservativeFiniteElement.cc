/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/fe/ConservativeFiniteElement.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/linalg/LinearAlgebra.h"
#include "eckit/linalg/Vector.h"
#include "eckit/log/Log.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/config/LibMir.h"
#include "mir/method/fe/FiniteElement.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"


namespace mir {
namespace method {
namespace fe {


ConservativeFiniteElement::ConservativeFiniteElement(const param::MIRParametrisation& param) : MethodWeighted(param) {
    std::string method;

    param.get("conservative-finite-element-method-input", method = "linear");
    inputMethod_.reset(FiniteElementFactory::build(method, "input", param));
    ASSERT(inputMethod_);

    inputMethod_->meshGeneratorParams().meshNodeLumpedMassMatrix_ = true;

    param.get("conservative-finite-element-method-output", method = "linear");
    outputMethod_.reset(FiniteElementFactory::build(method, "output", param));
    ASSERT(outputMethod_);

    outputMethod_->meshGeneratorParams().meshNodeLumpedMassMatrix_ = true;
}


ConservativeFiniteElement::~ConservativeFiniteElement() = default;


bool ConservativeFiniteElement::sameAs(const Method& other) const {
    auto o = dynamic_cast<const ConservativeFiniteElement*>(&other);
    return (o != nullptr) && inputMethod_->sameAs(*(o->inputMethod_)) && outputMethod_->sameAs(*(o->outputMethod_)) &&
           MethodWeighted::sameAs(*o);
}


void ConservativeFiniteElement::assemble(util::MIRStatistics& statistics, WeightMatrix& W,
                                         const repres::Representation& in, const repres::Representation& out) const {
    eckit::Channel& log = eckit::Log::debug<LibMir>();
    log << "ConservativeFiniteElement::assemble (input: " << in << ", output: " << out << ")" << std::endl;


    // 1) I: compute the matrix from input to output
    size_t Ni = out.numberOfPoints();
    size_t Nj = in.numberOfPoints();

    WeightMatrix I(Ni, Nj);
    inputMethod_->assemble(statistics, I, in, out);


    // 2) M_s: input (source) mesh lumped mass matrix
    auto Fin = inputMethod_->atlasMesh(statistics, in).nodes().field("node_lumped_mass_matrix");
    auto Min = atlas::array::make_view<double, 1>(Fin);

    ASSERT(0 < Nj && Nj <= size_t(Min.size()));
    eckit::linalg::Vector M_s(Min.data(), Nj);


    // 3) M_d^{-1}: output (destination) mesh lumped mass matrix inverse
    auto Fout = outputMethod_->atlasMesh(statistics, out).nodes().field("node_lumped_mass_matrix");
    auto Mout = atlas::array::make_view<double, 1>(Fout);

    ASSERT(0 < Ni && Ni <= size_t(Mout.size()));
    eckit::linalg::Vector inv_M_d(Ni);
    for (size_t i = 0; i < Ni; ++i) {
        inv_M_d[i] = eckit::types::is_approximately_equal(Mout[i], 0.) ? 0. : 1. / Mout[i];
    }


    // 4) W = M_d^{-1} I M_s
    W.reserve(I.rows(), I.cols(), I.nonZeros());  // reserve same space as I
    eckit::linalg::LinearAlgebra::backend().dsptd(inv_M_d, I, M_s, W);


    // 5) Normalise row weights
    auto it = W.begin();
    for (WeightMatrix::Size i = 0; i < W.rows(); ++i) {
        auto begin = it;
        auto end   = W.end(i);

        // accumulate and re-distribute (linearly) the row weights
        double sum = 0.;
        for (; it != end; ++it) {
            sum += *it;
        }

        if (!eckit::types::is_approximately_equal(sum, 0.)) {
            const double factor = 1. / sum;
            for (it = begin; it != end; ++it) {
                *it *= factor;
            }
        }
    }
}


const char* ConservativeFiniteElement::name() const {
    return "conservative-finite-element";
}


void ConservativeFiniteElement::hash(eckit::MD5& md5) const {
    MethodWeighted::hash(md5);
    inputMethod_->hash(md5);
    outputMethod_->hash(md5);
    md5.add(name());
}


int ConservativeFiniteElement::version() const {
    return 1;
}


void ConservativeFiniteElement::print(std::ostream& out) const {
    out << "ConservativeFiniteElement[inputMethod=";
    inputMethod_->print(out);
    out << ",outputMethod=";
    outputMethod_->print(out);
    out << ",";
    MethodWeighted::print(out);
    out << "]";
}


static MethodBuilder<ConservativeFiniteElement> __builder("conservative-finite-element");


}  // namespace fe
}  // namespace method
}  // namespace mir
