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

#include "eckit/log/JSON.h"
#include "eckit/utils/MD5.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/allocator/InPlaceAllocator.h"


namespace mir::method {


namespace {


#define ATLAS_METHOD(Type, Name)                                                                    \
    struct Type final : ProxyWeightedMethod {                                                       \
        explicit Type(const param::MIRParametrisation& param) : ProxyWeightedMethod(param, Name) {} \
    };

ATLAS_METHOD(StructuredBilinear, "structured-bilinear");
ATLAS_METHOD(StructuredBiquasicubic, "structured-biquasicubic");
ATLAS_METHOD(StructuredBicubic, "structured-bicubic");
ATLAS_METHOD(FiniteElement, "finite-element");
ATLAS_METHOD(ConservativeSphericalPolygon, "conservative-spherical-polygon");
ATLAS_METHOD(GridBoxAverage, "grid-box-average");
ATLAS_METHOD(GridBoxMaximum, "grid-box-maximum");

// "nearest-neighbour" (knn)
// "k-nearest-neighbours" (knn)
// "cubedsphere-bilinear"
// "regional-linear-2d" (structured)
// "spherical-vector"


#undef ATLAS_INTERPOL


const MethodFactory* MIR_METHODS[]{
    new MethodBuilder<StructuredBicubic>("atlas-structured-bicubic"),
    new MethodBuilder<StructuredBilinear>("atlas-structured-bilinear"),
    new MethodBuilder<StructuredBiquasicubic>("atlas-structured-biquasicubic"),
    new MethodBuilder<FiniteElement>("atlas-finite-element"),
    new MethodBuilder<ConservativeSphericalPolygon>("atlas-conservative-spherical-polygon"),
    new MethodBuilder<GridBoxAverage>("atlas-grid-box-average"),
    new MethodBuilder<GridBoxMaximum>("atlas-grid-box-maximum"),
};


}  // namespace


ProxyWeightedMethod::ProxyWeightedMethod(const param::MIRParametrisation& param,
                                         const std::string& interpolation_type) :
    MethodWeighted(param), type_(interpolation_type) {
    interpol_.set("matrix_free", false);
    interpol_.set("type", interpolation_type);
}


void ProxyWeightedMethod::foldSourceHalo(const atlas::Interpolation& interpol, size_t Nr, size_t Nc,
                                         WeightMatrix& W) const {
    ASSERT(Nr == W.rows());
    ASSERT(Nc < W.cols());

    const auto& fs = interpol.source();
    const auto global_index{atlas::array::make_view<atlas::gidx_t, 1>(fs.global_index())};
    ASSERT(global_index.size() == W.cols());

    auto* a = const_cast<eckit::linalg::Scalar*>(W.data());
    for (auto c = Nc; c < W.cols(); ++c) {
        ASSERT(1 <= global_index[c] && global_index[c] < Nc + 1);  // (global indexing is 1-based)
        a[global_index[c] - 1] += a[c];
        a[c] = 0.;
    }

    W.prune();
    WeightMatrix M(new util::allocator::InPlaceAllocator{
        Nr, Nc, W.nonZeros(), const_cast<eckit::linalg::Index*>(W.outer()),
        const_cast<eckit::linalg::Index*>(W.inner()), const_cast<eckit::linalg::Scalar*>(W.data())});
    W.swap(M);
}


const char* ProxyWeightedMethod::name() const {
    return type_.c_str();
}


int ProxyWeightedMethod::version() const {
    return 1;
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
           MethodWeighted::sameAs(*o);
}


void ProxyWeightedMethod::print(std::ostream& out) const {
    out << "ProxyWeightedMethod[interpolation=" << interpol_ << ",";
    MethodWeighted::print(out);
    out << "]";
}


void ProxyWeightedMethod::json(eckit::JSON& j) const {
    j.startObject();
    j << "interpolation" << interpol_.json(eckit::JSON::Formatting::compact());
    MethodWeighted::json(j);
    j.endObject();
}


void ProxyWeightedMethod::assemble(util::MIRStatistics&, WeightMatrix& W, const repres::Representation& in,
                                   const repres::Representation& out) const {
    // build matrix (with a halo on the source grid), move out of cache
    atlas::Interpolation interpol{interpol_, in.atlasGrid(), out.atlasGrid()};
    atlas::interpolation::MatrixCache cache{interpol};
    W.swap(const_cast<eckit::linalg::SparseMatrix&>(cache.matrix()));

    // fold source grid halo (from serial + halo into serial)
    foldSourceHalo(interpol, out.numberOfPoints(), in.numberOfPoints(), W);
}


}  // namespace mir::method
