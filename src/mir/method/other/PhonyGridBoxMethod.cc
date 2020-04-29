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


#include "mir/method/other/PhonyGridBoxMethod.h"

#include <algorithm>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "eckit/log/Seconds.h"
#include "eckit/log/Timer.h"
#include "eckit/utils/MD5.h"

#include "mir/action/context/Context.h"
#include "mir/api/Atlas.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"


namespace mir {
namespace method {
namespace other {


struct GridBoxAverageMB final : public PhonyGridBoxMethod {
    GridBoxAverageMB(const param::MIRParametrisation& param) : PhonyGridBoxMethod(param, "grid-box-average", false) {}
};


struct GridBoxAverageMF final : public PhonyGridBoxMethod {
    GridBoxAverageMF(const param::MIRParametrisation& param) : PhonyGridBoxMethod(param, "grid-box-average", true) {}
};


struct GridBoxMaximumMB final : public PhonyGridBoxMethod {
    GridBoxMaximumMB(const param::MIRParametrisation& param) : PhonyGridBoxMethod(param, "grid-box-maximum", false) {}
};


struct GridBoxMaximumMF final : public PhonyGridBoxMethod {
    GridBoxMaximumMF(const param::MIRParametrisation& param) : PhonyGridBoxMethod(param, "grid-box-maximum", true) {}
};


static MethodBuilder<GridBoxAverageMB> __method1("grid-box-average-matrix-based");
static MethodBuilder<GridBoxAverageMF> __method2("grid-box-average-matrix-free");
static MethodBuilder<GridBoxMaximumMB> __method3("grid-box-maximum-matrix-based");
static MethodBuilder<GridBoxMaximumMF> __method4("grid-box-maximum-matrix-free");


PhonyGridBoxMethod::PhonyGridBoxMethod(const param::MIRParametrisation& param, std::string type, bool matrixFree) :
    Method(param),
    type_(type),
    matrixFree_(matrixFree) {}


void PhonyGridBoxMethod::hash(eckit::MD5& md5) const {
    md5.add(type_);
    md5.add(matrixFree_);
}


void PhonyGridBoxMethod::execute(context::Context& ctx, const repres::Representation& in,
                                 const repres::Representation& out) const {
    eckit::Timer timer;
    auto& log   = eckit::Log::info();
    auto& field = ctx.field();

    struct Helper {
        Helper(const repres::Representation& r, const atlas::util::Config& config) :
            grid(r.atlasGrid()),
            n(grid.size()),
            fs(grid, config) {}

        void appendFieldCopy(const data::MIRValuesVector& values) {
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
    Helper input(in, atlas::option::halo(0));
    for (size_t i = 0; i < field.dimensions(); ++i) {
        input.appendFieldCopy(field.values(i));
    }
    log << type_ << ": set input... done, " << eckit::Seconds(timer.elapsed() - mark) << std::endl;


    log << type_ << ": set output" << std::endl;
    mark = timer.elapsed();
    Helper output(out, atlas::util::NoConfig());
    std::vector<data::MIRValuesVector> result(field.dimensions(), data::MIRValuesVector(output.n));
    for (auto& v : result) {
        output.appendFieldWrapped(v);
    }
    log << type_ << ": set output... done, " << eckit::Seconds(timer.elapsed() - mark) << std::endl;


    log << type_ << ": set interpolation..." << std::endl;
    mark        = timer.elapsed();
    auto config = atlas::util::Config("type", type_).set("matrix_free", matrixFree_);
    atlas::Interpolation interpol(config, input.grid, output.grid);
    log << type_ << ": set interpolation... done, " << eckit::Seconds(timer.elapsed() - mark) << std::endl;


    log << type_ << ": interpolate..." << std::endl;
    mark = timer.elapsed();
    interpol.execute(input.fields, output.fields);
    for (size_t i = 0; i < field.dimensions(); ++i) {
        field.update(result[i], i, true);
    }
    log << type_ << ": interpolate... done, " << eckit::Seconds(timer.elapsed() - mark) << std::endl;
}


bool PhonyGridBoxMethod::sameAs(const Method& other) const {
    auto o = dynamic_cast<const PhonyGridBoxMethod*>(&other);
    return (o != nullptr) && type_ == o->type_ && matrixFree_ == o->matrixFree_;
}


bool PhonyGridBoxMethod::canCrop() const {
    return true;
}


void PhonyGridBoxMethod::setCropping(const util::BoundingBox&) {
    NOTIMP;
}


bool PhonyGridBoxMethod::hasCropping() const {
    return false;
}


const util::BoundingBox& PhonyGridBoxMethod::getCropping() const {
    NOTIMP;
}


void PhonyGridBoxMethod::print(std::ostream& out) const {
    out << "GridBoxMethod[type=" << type_ << ",matrixFree=" << matrixFree_ << "]";
}


}  // namespace other
}  // namespace method
}  // namespace mir
