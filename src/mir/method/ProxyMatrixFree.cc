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


#include "mir/method/ProxyMatrixFree.h"

#include <algorithm>
#include <vector>

#include "eckit/utils/MD5.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Trace.h"


namespace mir {
namespace method {


struct StructuredBicubic final : public ProxyMatrixFree {
    explicit StructuredBicubic(const param::MIRParametrisation& param) : ProxyMatrixFree(param, "structured-bicubic") {}
};


struct StructuredBilinear final : public ProxyMatrixFree {
    explicit StructuredBilinear(const param::MIRParametrisation& param) :
        ProxyMatrixFree(param, "structured-bilinear") {}
};


struct StructuredBiquasicubic final : public ProxyMatrixFree {
    explicit StructuredBiquasicubic(const param::MIRParametrisation& param) :
        ProxyMatrixFree(param, "structured-biquasicubic") {}
};


struct GridBoxAverage final : public ProxyMatrixFree {
    explicit GridBoxAverage(const param::MIRParametrisation& param) : ProxyMatrixFree(param, "grid-box-average") {}
};


struct GridBoxMaximum final : public ProxyMatrixFree {
    explicit GridBoxMaximum(const param::MIRParametrisation& param) : ProxyMatrixFree(param, "grid-box-maximum") {}
};


static const MethodBuilder<StructuredBicubic> __method1("structured-bicubic-matrix-free");
static const MethodBuilder<StructuredBilinear> __method2("structured-bilinear-matrix-free");
static const MethodBuilder<StructuredBiquasicubic> __method3("structured-biquasicubic-matrix-free");
static const MethodBuilder<GridBoxAverage> __method4("grid-box-average-matrix-free");
static const MethodBuilder<GridBoxMaximum> __method5("grid-box-maximum-matrix-free");


static eckit::Hash::digest_t atlasOptionsDigest(const ProxyMatrixFree::atlas_config_t& options) {
    eckit::MD5 h;
    options.hash(h);
    return h.digest();
}


ProxyMatrixFree::ProxyMatrixFree(const param::MIRParametrisation& param, std::string type) : Method(param) {
    options_.set("type", type);
    options_.set("matrix_free", true);
}


void ProxyMatrixFree::hash(eckit::MD5& md5) const {
    md5.add(options_);
    md5.add(cropping_);
}


int ProxyMatrixFree::version() const {
    return 1;
}


void ProxyMatrixFree::execute(context::Context& ctx, const repres::Representation& in,
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

    trace::Timer timer("ProxyMatrixFree::execute");
    auto report = [](trace::Timer& timer, const std::string& msg) {
        timer.report(msg);
        timer.stop();
        timer.start();
    };

    auto& field = ctx.field();


    atlas::Interpolation interpol(options_, in.atlasGrid(), out.atlasGrid());
    Helper input(in.numberOfPoints(), interpol.source());
    Helper output(out.numberOfPoints(), interpol.target());
    report(timer, "set interpolation");


    for (size_t i = 0; i < field.dimensions(); ++i) {
        input.appendFieldCopy(field.values(i));
    }
    report(timer, "copy input");


    std::vector<MIRValuesVector> result(field.dimensions(), MIRValuesVector(output.n));
    for (auto& v : result) {
        output.appendFieldWrapped(v);
    }
    report(timer, "allocate output");


    interpol.execute(input.fields, output.fields);
    for (size_t i = 0; i < field.dimensions(); ++i) {
        field.update(result[i], i, true);
    }
    report(timer, "interpolate");
}


bool ProxyMatrixFree::sameAs(const Method& other) const {
    const auto* o = dynamic_cast<const ProxyMatrixFree*>(&other);
    return (o != nullptr) && atlasOptionsDigest(options_) == atlasOptionsDigest(o->options_) &&
           cropping_.sameAs(o->cropping_);
}


bool ProxyMatrixFree::canCrop() const {
    return true;
}


void ProxyMatrixFree::setCropping(const util::BoundingBox& bbox) {
    cropping_.boundingBox(bbox);
}


bool ProxyMatrixFree::hasCropping() const {
    return cropping_;
}


const util::BoundingBox& ProxyMatrixFree::getCropping() const {
    return cropping_.boundingBox();
}


void ProxyMatrixFree::print(std::ostream& out) const {
    out << "ProxyMatrixFree[options=" << options_ << ",cropping=" << cropping_ << "]";
}


}  // namespace method
}  // namespace mir
