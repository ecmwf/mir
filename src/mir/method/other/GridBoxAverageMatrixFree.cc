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


#include "mir/method/other/GridBoxAverageMatrixFree.h"

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


static MethodBuilder<GridBoxAverageMatrixFree> __method("grid-box-average-matrix-free");


GridBoxAverageMatrixFree::GridBoxAverageMatrixFree(const param::MIRParametrisation& param) : Method(param) {}


void GridBoxAverageMatrixFree::hash(eckit::MD5& md5) const {
    md5.add("grid-box-average-matrix-free");
}


void GridBoxAverageMatrixFree::execute(context::Context& ctx, const repres::Representation& in,
                                  const repres::Representation& out) const {
    eckit::Timer timer;
    auto& log   = eckit::Log::info();
    auto& field = ctx.field();

    struct Helper {
        Helper(const repres::Representation& r, const eckit::Configuration& config = atlas::util::NoConfig()) :
            grid(r.atlasGrid()),
            n(grid.size()),
            fs(grid, config) {}

        void appendFieldCopy(const data::MIRValuesVector& values) {
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


    log << "GridBoxAverageMatrixFree method: set input..." << std::endl;
    auto mark = timer.elapsed();
    Helper input(in, atlas::option::halo(0));
    for (size_t i = 0; i < field.dimensions(); ++i) {
        input.appendFieldCopy(field.values(i));
    }
    log << "GridBoxAverageMatrixFree method: set input... done, " << eckit::Seconds(timer.elapsed() - mark) << std::endl;


    log << "GridBoxAverageMatrixFree method: set output" << std::endl;
    mark = timer.elapsed();
    Helper output(out);
    std::vector<data::MIRValuesVector> result(field.dimensions(), data::MIRValuesVector(output.n));
    for (auto& v : result) {
        output.appendFieldWrapped(v);
    }
    log << "GridBoxAverageMatrixFree method: set output... done, " << eckit::Seconds(timer.elapsed() - mark) << std::endl;


    log << "GridBoxAverageMatrixFree method: set interpolation..." << std::endl;
    mark = timer.elapsed();
    auto config = atlas::util::Config("type", "grid-box-average").set("matrix_free", true);
    atlas::Interpolation interpol(config, input.grid, output.grid);
    log << "GridBoxAverageMatrixFree method: set interpolation... done, " << eckit::Seconds(timer.elapsed() - mark) << std::endl;


    log << "GridBoxAverageMatrixFree method: interpolate..." << std::endl;
    mark = timer.elapsed();
    interpol.execute(input.fields, output.fields);
    for (size_t i = 0; i < field.dimensions(); ++i) {
        field.update(result[i], i, true);
    }
    log << "GridBoxAverageMatrixFree method: interpolate... done, " << eckit::Seconds(timer.elapsed() - mark) << std::endl;
}


bool GridBoxAverageMatrixFree::sameAs(const Method& other) const {
    auto o = dynamic_cast<const GridBoxAverageMatrixFree*>(&other);
    return (o != nullptr);
}


bool GridBoxAverageMatrixFree::canCrop() const {
    return true;
}


void GridBoxAverageMatrixFree::setCropping(const util::BoundingBox&) {
    NOTIMP;
}


bool GridBoxAverageMatrixFree::hasCropping() const {
    return false;
}


const util::BoundingBox& GridBoxAverageMatrixFree::getCropping() const {
    NOTIMP;
}


void GridBoxAverageMatrixFree::print(std::ostream& out) const {
    out << "GridBoxAverageMatrixFreeMethod[]";
}


}  // namespace other
}  // namespace method
}  // namespace mir
