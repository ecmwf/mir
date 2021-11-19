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


#include "mir/method/ProxyMethod.h"

#include <algorithm>
#include <vector>

#include "eckit/utils/MD5.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Trace.h"


namespace mir {
namespace method {


struct StructuredBicubic final : public ProxyMethod {
    explicit StructuredBicubic(const param::MIRParametrisation& param) : ProxyMethod(param, "structured-bicubic") {}
};


struct StructuredBilinear final : public ProxyMethod {
    explicit StructuredBilinear(const param::MIRParametrisation& param) : ProxyMethod(param, "structured-bilinear") {}
};


struct StructuredBiquasicubic final : public ProxyMethod {
    explicit StructuredBiquasicubic(const param::MIRParametrisation& param) :
        ProxyMethod(param, "structured-biquasicubic") {}
};


struct GridBoxAverage final : public ProxyMethod {
    explicit GridBoxAverage(const param::MIRParametrisation& param) : ProxyMethod(param, "grid-box-average") {}
};


struct GridBoxMaximum final : public ProxyMethod {
    explicit GridBoxMaximum(const param::MIRParametrisation& param) : ProxyMethod(param, "grid-box-maximum") {}
};


static MethodBuilder<StructuredBicubic> __method1("structured-bicubic");
static MethodBuilder<StructuredBilinear> __method2("structured-bilinear");
static MethodBuilder<StructuredBiquasicubic> __method3("structured-biquasicubic");
static MethodBuilder<GridBoxAverage> __method4("grid-box-average-matrix-free");
static MethodBuilder<GridBoxMaximum> __method5("grid-box-maximum-matrix-free");


static eckit::Hash::digest_t atlasOptionsDigest(const ProxyMethod::atlas_config_t& options) {
    eckit::MD5 h;
    options.hash(h);
    return h.digest();
}


ProxyMethod::ProxyMethod(const param::MIRParametrisation& param, std::string type) :
    Method(param), type_(std::move(type)) {

    // // "interpolation" should return one of the methods registered above
    // param.get("interpolation", type_);
    // ASSERT(!type_.empty());

    // NOTE: while the Atlas-built matrix is unavailable, "grid-box-*" is inconsistent (default should be false)
    bool matrixFree = true;
    param.get("interpolation-matrix-free", matrixFree);

    options_ = {"type", type_};
    options_.set("matrix_free", matrixFree);
}


ProxyMethod::~ProxyMethod() = default;


void ProxyMethod::hash(eckit::MD5& md5) const {
    md5.add(options_);
    md5.add(cropping_);
}


int ProxyMethod::version() const {
    return 0;
}


void ProxyMethod::execute(context::Context& ctx, const repres::Representation& in,
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

    trace::Timer timer("ProxyMethod::execute", Log::info());
    auto report = [](trace::Timer& timer, const std::string& msg) {
        timer.report(msg);
        timer.stop();
        timer.start();
    };

    auto& field = ctx.field();


    atlas::Interpolation interpol(options_, in.atlasGrid(), out.atlasGrid());
    Helper input(in.numberOfPoints(), interpol.source());
    Helper output(out.numberOfPoints(), interpol.target());
    report(timer, type_ + ": set interpolation");


    for (size_t i = 0; i < field.dimensions(); ++i) {
        input.appendFieldCopy(field.values(i));
    }
    report(timer, type_ + ": copy input");


    std::vector<MIRValuesVector> result(field.dimensions(), MIRValuesVector(output.n));
    for (auto& v : result) {
        output.appendFieldWrapped(v);
    }
    report(timer, type_ + ": allocate output");


    interpol.execute(input.fields, output.fields);
    for (size_t i = 0; i < field.dimensions(); ++i) {
        field.update(result[i], i, true);
    }
    report(timer, type_ + ": interpolate");
}


bool ProxyMethod::sameAs(const Method& other) const {
    auto o = dynamic_cast<const ProxyMethod*>(&other);
    return (o != nullptr) && atlasOptionsDigest(options_) == atlasOptionsDigest(o->options_) &&
           cropping_.sameAs(o->cropping_);
}


bool ProxyMethod::canCrop() const {
    return true;
}


void ProxyMethod::setCropping(const util::BoundingBox& bbox) {
    cropping_.boundingBox(bbox);
}


bool ProxyMethod::hasCropping() const {
    return cropping_;
}


const util::BoundingBox& ProxyMethod::getCropping() const {
    return cropping_.boundingBox();
}


void ProxyMethod::print(std::ostream& out) const {
    out << "ProxyMethod[options=" << options_ << ",cropping=" << cropping_ << "]";
}


}  // namespace method
}  // namespace mir
