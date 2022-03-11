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


#include "mir/method/ProxyMatrixBased.h"

#include <algorithm>
#include <vector>

#include "eckit/utils/MD5.h"

// #include "mir/action/context/Context.h"
// #include "mir/data/MIRField.h"
// #include "mir/repres/Representation.h"
// #include "mir/util/Exceptions.h"
// #include "mir/util/Trace.h"


namespace mir {
namespace method {


struct GridBoxAverage final : public ProxyMatrixBased {
    explicit GridBoxAverage(const param::MIRParametrisation& param) : ProxyMatrixBased(param, "grid-box-average") {}
};


struct GridBoxMaximum final : public ProxyMatrixBased {
    explicit GridBoxMaximum(const param::MIRParametrisation& param) : ProxyMatrixBased(param, "grid-box-maximum") {}
};


static const MethodBuilder<GridBoxAverage> __method1("grid-box-average");
static const MethodBuilder<GridBoxMaximum> __method2("grid-box-maximum");


static eckit::Hash::digest_t atlasOptionsDigest(const ProxyMatrixBased::atlas_config_t& options) {
    eckit::MD5 h;
    options.hash(h);
    return h.digest();
}


ProxyMatrixBased::ProxyMatrixBased(const param::MIRParametrisation& param, std::string type) :
    Method(param) {
    options_.set("type", type);
    options_.set("matrix_free", false);
}


void ProxyMatrixBased::hash(eckit::MD5& md5) const {
    md5.add(options_);
    md5.add(cropping_);
}


int ProxyMatrixBased::version() const {
    return 0;
}


void ProxyMatrixBased::execute(context::Context& ctx, const repres::Representation& in,
                          const repres::Representation& out) const {
    struct Helper {
        Helper(size_t numberOfPoints, atlas::FunctionSpace fspace) : n(numberOfPoints), fs(fspace) {}

        void appendFieldCopy(const MIRValuesVector& values) {
            ASSERT(n == values.size());
            auto view = atlas::array::make_view<double, 1>(fields.add(fs.createField<double>()));
            ASSERT(view.contiguous());
            ASSERT(values.size() <= size_t(view.size()));
            std::copy_n(values.begin(), n, view.data());
        }

        void appendFieldWrapped(MIRValuesVector& values) {
            ASSERT(n == values.size());
            auto field = fields.add(atlas::Field("?", values.data(), atlas::array::make_shape(n)));
            field.set_functionspace(fs);
        }

        const size_t n;
        atlas::FunctionSpace fs;
        atlas::FieldSet fields;
    };

    NOTIMP;
}


bool ProxyMatrixBased::sameAs(const Method& other) const {
    const auto* o = dynamic_cast<const ProxyMatrixBased*>(&other);
    return (o != nullptr) && atlasOptionsDigest(options_) == atlasOptionsDigest(o->options_) &&
           cropping_.sameAs(o->cropping_);
}


bool ProxyMatrixBased::canCrop() const {
    return true;
}


void ProxyMatrixBased::setCropping(const util::BoundingBox& bbox) {
    cropping_.boundingBox(bbox);
}


bool ProxyMatrixBased::hasCropping() const {
    return cropping_;
}


const util::BoundingBox& ProxyMatrixBased::getCropping() const {
    return cropping_.boundingBox();
}


void ProxyMatrixBased::print(std::ostream& out) const {
    out << "ProxyMatrixBased[options=" << options_ << ",cropping=" << cropping_ << "]";
}


}  // namespace method
}  // namespace mir
