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


#include "mir/method/PhonyMethod.h"

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


struct StructuredBicubicMB final : public PhonyMethod {
    StructuredBicubicMB(const param::MIRParametrisation& param) : PhonyMethod(param, 2, false) {}
};


struct StructuredBicubicMF final : public PhonyMethod {
    StructuredBicubicMF(const param::MIRParametrisation& param) : PhonyMethod(param, 2, true) {}
};


struct StructuredBilinearMB final : public PhonyMethod {
    StructuredBilinearMB(const param::MIRParametrisation& param) : PhonyMethod(param, 1, false) {}
};


struct StructuredBilinearMF final : public PhonyMethod {
    StructuredBilinearMF(const param::MIRParametrisation& param) : PhonyMethod(param, 1, true) {}
};


struct StructuredQuasiCubicMB final : public PhonyMethod {
    StructuredQuasiCubicMB(const param::MIRParametrisation& param) : PhonyMethod(param, 2, false) {}
};


struct StructuredQuasiCubicMF final : public PhonyMethod {
    StructuredQuasiCubicMF(const param::MIRParametrisation& param) : PhonyMethod(param, 2, true) {}
};


static MethodBuilder<StructuredBicubicMB> __method1("structured-bicubic");
static MethodBuilder<StructuredBicubicMF> __method2("structured-bicubic-matrix-free");
static MethodBuilder<StructuredBilinearMB> __method3("structured-bilinear");
static MethodBuilder<StructuredBilinearMF> __method4("structured-bilinear-matrix-free");
static MethodBuilder<StructuredQuasiCubicMB> __method5("structured-quasicubic");
static MethodBuilder<StructuredQuasiCubicMF> __method6("structured-quasicubic-matrix-free");


static eckit::Hash::digest_t atlasOptionsDigest(const PhonyMethod::atlas_config_t& options) {
    eckit::MD5 h;
    options.hash(h);
    return h.digest();
}


PhonyMethod::PhonyMethod(const param::MIRParametrisation& param, size_t halo, bool matrixFree) : Method(param) {

    // "interpolation" should return one of the methods registered above
    param.get("interpolation", type_);
    ASSERT(!type_.empty());

    options_ = {"type", type_};
    options_.set("halo", halo);
    options_.set("matrix_free", matrixFree);
}


void PhonyMethod::hash(eckit::MD5& md5) const {
    md5.add(options_);
    md5.add(cropping_);
}


void PhonyMethod::execute(context::Context& ctx, const repres::Representation& in,
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
    mark = timer.elapsed();
    atlas::Interpolation interpol(options_, input.fs, output.fs);
    interpol.execute(input.fields, output.fields);

    for (size_t i = 0; i < field.dimensions(); ++i) {
        field.update(result[i], i, true);
    }
    log << type_ << ": interpolate... done, " << eckit::Seconds(timer.elapsed() - mark) << std::endl;
}


bool PhonyMethod::sameAs(const Method& other) const {
    auto o = dynamic_cast<const PhonyMethod*>(&other);
    return (o != nullptr) && atlasOptionsDigest(options_) == atlasOptionsDigest(o->options_) &&
           cropping_ == o->cropping_;
}


bool PhonyMethod::canCrop() const {
    return true;
}


void PhonyMethod::setCropping(const util::BoundingBox& bbox) {
    cropping_.boundingBox(bbox);
}


bool PhonyMethod::hasCropping() const {
    return cropping_;
}


const util::BoundingBox& PhonyMethod::getCropping() const {
    return cropping_.boundingBox();
}


void PhonyMethod::print(std::ostream& out) const {
    out << "Method[options=" << options_ << ",cropping=" << cropping_ << "]";
}


}  // namespace method
}  // namespace mir
