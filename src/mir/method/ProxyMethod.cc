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

#include "eckit/log/JSON.h"
#include "eckit/utils/MD5.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Trace.h"


namespace mir::method {


const std::string STRUCTURED_BICUBIC{"structured-bicubic"};
const std::string STRUCTURED_BILINEAR{"structured-bilinear"};
const std::string STRUCTURED_BIQUASICUBIC{"structured-biquasicubic"};
const std::string GRID_BOX_AVERAGE{"grid-box-average-matrix-free"};
const std::string GRID_BOX_MAXIMUM{"grid-box-maximum-matrix-free"};


auto remove_matrix_free = [](const std::string& s) {
    const std::string suffix{"-matrix-free"};
    if (s.size() >= suffix.size() && s.compare(s.size() - suffix.size(), suffix.size(), suffix) == 0) {
        std::string t{s};
        t.erase(s.size() - suffix.size());
        return t;
    }
    return s;
};


struct StructuredBicubic final : public ProxyMethod {
    explicit StructuredBicubic(const param::MIRParametrisation& param) : ProxyMethod(param, STRUCTURED_BICUBIC) {}
    const char* type() const override { return STRUCTURED_BICUBIC.c_str(); }
};


struct StructuredBilinear final : public ProxyMethod {
    explicit StructuredBilinear(const param::MIRParametrisation& param) : ProxyMethod(param, STRUCTURED_BILINEAR) {}
    const char* type() const override { return STRUCTURED_BILINEAR.c_str(); }
};


struct StructuredBiquasicubic final : public ProxyMethod {
    explicit StructuredBiquasicubic(const param::MIRParametrisation& param) :
        ProxyMethod(param, STRUCTURED_BIQUASICUBIC) {}
    const char* type() const override { return STRUCTURED_BIQUASICUBIC.c_str(); }
};


struct GridBoxAverage final : public ProxyMethod {
    explicit GridBoxAverage(const param::MIRParametrisation& param) :
        ProxyMethod(param, remove_matrix_free(GRID_BOX_AVERAGE)) {}
    const char* type() const override { return GRID_BOX_AVERAGE.c_str(); }
};


struct GridBoxMaximum final : public ProxyMethod {
    explicit GridBoxMaximum(const param::MIRParametrisation& param) :
        ProxyMethod(param, remove_matrix_free(GRID_BOX_MAXIMUM)) {}
    const char* type() const override { return GRID_BOX_MAXIMUM.c_str(); }
};


static const MethodBuilder<StructuredBicubic> __method1(STRUCTURED_BICUBIC);
static const MethodBuilder<StructuredBilinear> __method2(STRUCTURED_BILINEAR);
static const MethodBuilder<StructuredBiquasicubic> __method3(STRUCTURED_BIQUASICUBIC);
static const MethodBuilder<GridBoxAverage> __method4(GRID_BOX_AVERAGE);
static const MethodBuilder<GridBoxMaximum> __method5(GRID_BOX_MAXIMUM);


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

    trace::Timer timer("ProxyMethod::execute");
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
    const auto* o = dynamic_cast<const ProxyMethod*>(&other);
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


void ProxyMethod::json(eckit::JSON& out) const {
    out << type_;
    out << "interpolation-matrix-free" << options_.getBool("matrix_free");
}


}  // namespace mir::method
