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


#include "mir/method/StructuredMethod2D.h"

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


static MethodBuilder<StructuredMethod2D> __method1("structured-bicubic");
static MethodBuilder<StructuredMethod2D> __method2("structured-bilinear");
static MethodBuilder<StructuredMethod2D> __method3("structured-biquasicubic");


StructuredMethod2D::StructuredMethod2D(const param::MIRParametrisation& param) : Method(param) {

    // "interpolation" should return one of the methods registered above
    param.get("interpolation", method_);
    ASSERT(!method_.empty());

    const static struct {
        const char* method;
        size_t halo;
    } halos[] = {{"structured-bilinear", 1}, {"structured-bicubic", 2}, {"structured-biquasicubic", 2}};

    halo_ = 0;
    for (auto& h : halos) {
        if (method_ == h.method) {
            halo_ = h.halo;
            break;
        }
    }
}


void StructuredMethod2D::hash(eckit::MD5& md5) const {
    md5.add(method_);
    md5.add(cropping_);
}


void StructuredMethod2D::execute(context::Context& ctx, const repres::Representation& in,
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
            std::copy(values.begin(), values.end(), view.data());
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


    log << "Structured method: set input..." << std::endl;
    auto mark = timer.elapsed();
    Helper input(in, atlas::option::halo(halo_));
    for (size_t i = 0; i < field.dimensions(); ++i) {
        input.appendFieldCopy(field.values(i));
    }
    log << "Structured method: set input... done, " << eckit::Seconds(timer.elapsed() - mark) << std::endl;


    log << "Structured method: set output" << std::endl;
    mark = timer.elapsed();
    Helper output(out);
    std::vector<data::MIRValuesVector> result(field.dimensions(), data::MIRValuesVector(output.n));
    for (auto& v : result) {
        output.appendFieldWrapped(v);
    }
    log << "Structured method: set output... done, " << eckit::Seconds(timer.elapsed() - mark) << std::endl;


    log << "Structured method: interpolate..." << std::endl;
    mark = timer.elapsed();
    atlas::util::Config config("type", method_);
    config.set("matrix_free", true);

    atlas::Interpolation interpol(config, input.fs, output.fs);
    interpol.execute(input.fields, output.fields);

    for (size_t i = 0; i < field.dimensions(); ++i) {
        field.update(result[i], i, true);
    }
    log << "Structured method: interpolate... done, " << eckit::Seconds(timer.elapsed() - mark) << std::endl;
}


bool StructuredMethod2D::sameAs(const Method& other) const {
    auto o = dynamic_cast<const StructuredMethod2D*>(&other);
    return o && method_ == o->method_;
}


bool StructuredMethod2D::canCrop() const {
    return true;
}


void StructuredMethod2D::setCropping(const util::BoundingBox& bbox) {
    cropping_.boundingBox(bbox);
}


bool StructuredMethod2D::hasCropping() const {
    return cropping_;
}


const util::BoundingBox& StructuredMethod2D::getCropping() const {
    return cropping_.boundingBox();
}


void StructuredMethod2D::print(std::ostream& out) const {
    out << "StructuredMethod[method=" << method() << ",halo=" << halo_ << "]";
}


}  // namespace method
}  // namespace mir
