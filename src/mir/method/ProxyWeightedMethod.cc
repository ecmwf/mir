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


namespace mir::method {


struct StructuredBicubic final : public ProxyWeightedMethod {
    explicit StructuredBicubic(const param::MIRParametrisation& param) :
        ProxyWeightedMethod(param, "structured-bicubic") {}
};


static const MethodFactory* METHODS[]{
    new MethodBuilder<StructuredBicubic>("structured-bicubic"),
};


ProxyWeightedMethod::ProxyWeightedMethod(const param::MIRParametrisation& param, const std::string& type) :
    MethodWeighted(param), type_(type) {
    options_ = {"type", type};
    options_.set("matrix_free", false);
}


const char* ProxyWeightedMethod::name() const {
    return type_.c_str();
}


int ProxyWeightedMethod::version() const {
    return -1;
}


void ProxyWeightedMethod::hash(eckit::MD5& h) const {
    MethodWeighted::hash(h);
    h.add(options_);
}


bool ProxyWeightedMethod::sameAs(const Method& other) const {
    auto digest = [](const auto& options) {
        eckit::MD5 h;
        h.add(options);
        return h.digest();
    };

    const auto* o = dynamic_cast<const ProxyWeightedMethod*>(&other);
    return (o != nullptr) && name() == std::string{o->name()} && digest(options_) == digest(o->options_) &&
           MethodWeighted::sameAs(*o);
}


void ProxyWeightedMethod::print(std::ostream& out) const {
    out << "ProxyWeightedMethod[options=" << options_ << ",";
    MethodWeighted::print(out);
    out << "]";
}


void ProxyWeightedMethod::json(eckit::JSON& j) const {
    j.startObject();
    j << "options" << options_.json(eckit::JSON::Formatting::compact());
    MethodWeighted::json(j);
    j.endObject();
}


void ProxyWeightedMethod::assemble(util::MIRStatistics&, WeightMatrix& W, const repres::Representation& in,
                                   const repres::Representation& out) const {
    atlas::Interpolation interpol(options_, in.atlasGrid(), out.atlasGrid());

    auto cache = interpol.createCache();
    ASSERT(cache);

    const auto* entry = dynamic_cast<const atlas::interpolation::MatrixCacheEntry*>(cache.get("Matrix"));
    ASSERT(entry != nullptr);

    W.swap(const_cast<eckit::linalg::SparseMatrix&>(entry->matrix()));

    auto& fs = interpol.source();
    const auto gi{atlas::array::make_view<atlas::gidx_t, 1>(fs.global_index())};

    atlas::gidx_t min = std::numeric_limits<atlas::gidx_t>::max();
    atlas::gidx_t max = std::numeric_limits<atlas::gidx_t>::min();
    for (size_t i = 0; i < gi.size(); ++i) {
        min = std::min(min, gi(i));
        max = std::max(max, gi(i));
    }

    std::cout << "min: " << min << std::endl;
    std::cout << "max: " << max << std::endl;


    auto Nr   = W.rows();
    auto Nc   = W.cols();
    auto Nin  = in.numberOfPoints();
    auto Nout = out.numberOfPoints();
    ASSERT(Nr == Nout);
    ASSERT(Nc == Nin);
}


bool ProxyWeightedMethod::validateMatrixWeights() const {
    return true;
}


}  // namespace mir::method
