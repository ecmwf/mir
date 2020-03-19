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


#include "mir/action/filter/DerivativeFilter.h"

#include <iostream>
#include <memory>

#include "eckit/exception/Exceptions.h"

#include "mir/action/context/Context.h"
#include "mir/api/Atlas.h"
#include "mir/api/MIREstimation.h"
#include "mir/caching/InMemoryMeshCache.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/MIRStatistics.h"


namespace mir {
namespace action {


DerivativeFilter::DerivativeFilter(const param::MIRParametrisation& param) :
    Action(param),
    meshGeneratorParams_("input", param) {}


DerivativeFilter::~DerivativeFilter() = default;


bool DerivativeFilter::sameAs(const Action& other) const {
    auto o = dynamic_cast<const DerivativeFilter*>(&other);
    return (o != nullptr);
}


void DerivativeFilter::print(std::ostream& out) const {
    out << "DerivativeFilter[meshGeneratorParameters=" << meshGeneratorParams_ << "]";
}


void DerivativeFilter::execute(context::Context& ctx) const {
    auto timing(ctx.statistics().derivativeTimer());


    // Generate supporting mesh, disabling incompatible generator features
    auto& field = ctx.field();
    if (field.hasMissing()) {
        throw eckit::UserError("DerivativeFilter: missing values not supported");
    }

    auto levels = field.dimensions();
    auto grid   = field.representation()->atlasGrid();
    auto points = size_t(grid.size());

    auto params = meshGeneratorParams_;
    field.representation()->fill(params);

    params.set("3d", false);
    params.set("force_include_north_pole", false);
    params.set("force_include_south_pole", false);

    auto mesh = caching::InMemoryMeshCache::atlasMesh(ctx.statistics(), grid, params);


    // Construct Nabla via FVM
    atlas::numerics::fvm::Method fvm(mesh);
    atlas::numerics::Nabla nabla(fvm);


    // Set input/output fields (copying is not great, but there you go)
    auto scalar = fvm.node_columns().createField<double>(atlas::option::levels(levels));
    auto grad   = fvm.node_columns().createField<double>(atlas::option::levels(levels) | atlas::option::variables(2));

    auto nodes = fvm.node_columns().nodes().size();
    const atlas::mesh::IsGhostNode is_ghost(fvm.node_columns().nodes());

    auto vScalar = atlas::array::make_view<double, 2>(scalar);
    ASSERT(points <= size_t(vScalar.shape(0)));

    for (size_t l = 0; l < levels; ++l) {
        auto& values = field.direct(l);
        ASSERT(points == values.size());

        size_t m = 0;
        for (atlas::idx_t n = 0; n < nodes; ++n) {
            vScalar(n, l) = is_ghost(n) ? 0. : values[m++];
        }
        ASSERT(points == m);
    }

    scalar.haloExchange();


    // Calculate gradients
    nabla.gradient(scalar, grad);


    // Set results
    auto vGrad = atlas::array::make_view<double, 3>(grad);
    ASSERT(points <= size_t(vGrad.shape(0)));

    for (size_t l = 0, d = 0; l < levels; ++l) {
        for (auto ll : {atlas::LON, atlas::LAT}) {
            data::MIRValuesVector values(points);

            size_t m = 0;
            for (atlas::idx_t n = 0; n < nodes; ++n) {
                if (!is_ghost(n)) {
                    values[m++] = vGrad(n, l, ll);
                }
            }
            ASSERT(points == m);

            field.update(values, d++);
        }
    }
}


const char* DerivativeFilter::name() const {
    return "DerivativeFilter";
}


void DerivativeFilter::estimate(context::Context&, api::MIREstimation& estimation) const {
    estimation.sameAsInput();
}


static ActionBuilder<DerivativeFilter> __filter("filter.derivative");


}  // namespace action
}  // namespace mir
