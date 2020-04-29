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

#include "eckit/exception/Exceptions.h"
#include "eckit/utils/MD5.h"

#include "eckit/log/Log.h"
#include "eckit/log/Seconds.h"
#include "eckit/log/Timer.h"

#include "mir/action/context/Context.h"
#include "mir/api/Atlas.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"


namespace mir {
namespace method {


using param_t = param::MIRParametrisation;


struct StructuredBicubic final : public ProxyMethod {
    StructuredBicubic(const param_t& param) : ProxyMethod(param, "structured-bicubic", 2, false) {}
};


struct StructuredBilinear final : public ProxyMethod {
    StructuredBilinear(const param_t& param) : ProxyMethod(param, "structured-bilinear", 1, false) {}
};


struct StructuredBiquasicubic final : public ProxyMethod {
    StructuredBiquasicubic(const param_t& param) : ProxyMethod(param, "structured-biquasicubic", 2, false) {}
};


struct GridBoxAverage final : public ProxyMethod {
    GridBoxAverage(const param_t& param) : ProxyMethod(param, "grid-box-average", 0, true) {}
};


struct GridBoxMaximum final : public ProxyMethod {
    GridBoxMaximum(const param_t& param) : ProxyMethod(param, "grid-box-maximum", 0, true) {}
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


ProxyMethod::ProxyMethod(const param::MIRParametrisation& param, std::string type, size_t halo, bool setupUsingGrids) :
    type_(type),
    Method(param) {

    // // "interpolation" should return one of the methods registered above
    // param.get("interpolation", type_);
    // ASSERT(!type_.empty());

    // NOTE: until interface to Atlas-built matrix is not available, "grid-box-average"/"grid-box-maximum" is
    // inconsistent while the situation isn't resolved (here the default should be false)
    bool matrixFree = true;
    param.get("interpolation-matrix-free", matrixFree);

    options_ = {"type", type_};
    options_.set("halo", halo);
    options_.set("matrix_free", matrixFree);
    options_.set("setup_using_grids", setupUsingGrids);
}


void ProxyMethod::hash(eckit::MD5& md5) const {
    md5.add(options_);
    md5.add(cropping_);
}


void ProxyMethod::execute(context::Context& ctx, const repres::Representation& in,
                          const repres::Representation& out) const {
    eckit::Timer timer;
    auto& log   = eckit::Log::info();
    auto& field = ctx.field();

    struct Helper {
        Helper(const repres::Representation& r, const eckit::Configuration& config = atlas::util::NoConfig()) :
            grid(r.atlasGrid()),
            n(grid.size()),
            fs(grid, config) {}

        void appendFieldCopy(const MIRValuesVector& values) {
            ASSERT(n == values.size());
            auto view = atlas::array::make_view<double, 1>(fields.add(fs.createField<double>()));
            ASSERT(view.contiguous());
            ASSERT(values.size() <= size_t(view.size()));
            std::copy_n(values.begin(), n, view.data());
        }

        void appendFieldWrapped(data::MIRValuesVector& values) {
            ASSERT(n == values.size());
            auto field = fields.add(atlas::Field("?", values.data(), atlas::array::make_shape(n)));
            field.set_functionspace(fs);
        }

        const atlas::Grid grid;
        const size_t n;
        atlas::functionspace::StructuredColumns fs;
        atlas::FieldSet fields;
    };


    log << type_ << ": set input..." << std::endl;
    auto mark = timer.elapsed();
    Helper input(in, atlas::option::halo(options_.getUnsigned("halo")));
    for (size_t i = 0; i < field.dimensions(); ++i) {
        input.appendFieldCopy(field.values(i));
    }
    log << type_ << ": set input... done, " << eckit::Seconds(timer.elapsed() - mark) << std::endl;


    log << type_ << ": set output" << std::endl;
    mark = timer.elapsed();
    Helper output(out);
    std::vector<data::MIRValuesVector> result(field.dimensions(), data::MIRValuesVector(output.n));
    for (auto& v : result) {
        output.appendFieldWrapped(v);
    }
    log << type_ << ": set output... done, " << eckit::Seconds(timer.elapsed() - mark) << std::endl;


    log << type_ << ": interpolate..." << std::endl;
    mark          = timer.elapsed();
    auto interpol = options_.getBool("setup_using_grids") ? atlas::Interpolation(options_, input.grid, output.grid)
                                                          : atlas::Interpolation(options_, input.fs, output.fs);
    interpol.execute(input.fields, output.fields);

    for (size_t i = 0; i < field.dimensions(); ++i) {
        field.update(result[i], i, true);
    }
    log << type_ << ": interpolate... done, " << eckit::Seconds(timer.elapsed() - mark) << std::endl;
}


bool ProxyMethod::sameAs(const Method& other) const {
    auto o = dynamic_cast<const ProxyMethod*>(&other);
    return (o != nullptr) && atlasOptionsDigest(options_) == atlasOptionsDigest(o->options_) &&
           cropping_ == o->cropping_;
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
