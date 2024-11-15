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


#include "mir/method/ProxyWeightedMethod.h"

#include <ostream>
#include <vector>

#include "eckit/log/JSON.h"
#include "eckit/utils/MD5.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/allocator/InPlaceAllocator.h"


namespace mir::method {


namespace {


struct StructuredBicubic final : public ProxyWeightedMethod {
    explicit StructuredBicubic(const param::MIRParametrisation& param) :
        ProxyWeightedMethod(param, "structured-bicubic", "serial-halo to serial") {}
};


}  // namespace


static const MethodFactory* METHODS[]{
    new MethodBuilder<StructuredBicubic>("structured-bicubic"),
};


ProxyWeightedMethod::ProxyWeightedMethod(const param::MIRParametrisation& param, const std::string& interpolation_type,
                                         const std::string& renumber_type) :
    MethodWeighted(param), type_(interpolation_type) {
    interpol_.set("matrix_free", false);
    interpol_.set("type", interpolation_type);
    renumber_.set("type", renumber_type);
}


const char* ProxyWeightedMethod::name() const {
    return type_.c_str();
}


int ProxyWeightedMethod::version() const {
    return -1;
}


void ProxyWeightedMethod::hash(eckit::MD5& h) const {
    MethodWeighted::hash(h);
    h.add(interpol_);
}


bool ProxyWeightedMethod::sameAs(const Method& other) const {
    auto digest = [](const auto& config) {
        eckit::MD5 h;
        config.hash(h);
        return h.digest();
    };

    const auto* o = dynamic_cast<const ProxyWeightedMethod*>(&other);
    return (o != nullptr) && name() == std::string{o->name()} && digest(interpol_) == digest(o->interpol_) &&
           digest(renumber_) == digest(o->renumber_) && MethodWeighted::sameAs(*o);
}


void ProxyWeightedMethod::print(std::ostream& out) const {
    out << "ProxyWeightedMethod[interpolation=" << interpol_ << ",renumber=" << renumber_ << ",";
    MethodWeighted::print(out);
    out << "]";
}


void ProxyWeightedMethod::json(eckit::JSON& j) const {
    j.startObject();
    j << "options" << interpol_.json(eckit::JSON::Formatting::compact());
    MethodWeighted::json(j);
    j.endObject();
}


void ProxyWeightedMethod::assemble(util::MIRStatistics&, WeightMatrix& W, const repres::Representation& in,
                                   const repres::Representation& out) const {
    // interpolation matrix build and assign (with a halo on the source grid)
    atlas::Interpolation interpol{interpol_, in.atlasGrid(), out.atlasGrid()};
    {
        atlas::interpolation::MatrixCache cache{interpol};
        const auto& M = cache.matrix();
        W.swap(const_cast<eckit::linalg::SparseMatrix&>(M));
    }

    // fold serial+halo into serial
    const auto Nr = out.numberOfPoints();
    const auto Nc = in.numberOfPoints();
    ASSERT(Nr == W.rows());
    ASSERT(Nc < W.cols());

    {
        const auto& fs = interpol.source();
        const auto gidx{atlas::array::make_view<atlas::gidx_t, 1>(fs.global_index())};

        auto* a = const_cast<eckit::linalg::Scalar*>(W.data());
        for (auto c = Nc; c < W.cols(); ++c) {
            ASSERT(1 <= gidx[c] && gidx[c] < Nc + 1);  // (global indexing is 1-based)
            a[gidx[c] - 1] += a[c];
            a[c] = 0.;
        }
        W.prune();
    }

    // reshape matrix
    WeightMatrix M(new util::allocator::InPlaceAllocator{
        Nr, Nc, W.nonZeros(), const_cast<eckit::linalg::Index*>(W.outer()),
        const_cast<eckit::linalg::Index*>(W.inner()), const_cast<eckit::linalg::Scalar*>(W.data())});
    W.swap(M);
}


bool ProxyWeightedMethod::validateMatrixWeights() const {
    return true;
}


}  // namespace mir::method
