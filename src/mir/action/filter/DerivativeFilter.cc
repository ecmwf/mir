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
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/MIRStatistics.h"


namespace mir {
namespace action {


DerivativeFilter::DerivativeFilter(const param::MIRParametrisation& param) : Action(param) {
    param.get("derivative-method", method_ = "linear");
}


DerivativeFilter::~DerivativeFilter() = default;


bool DerivativeFilter::sameAs(const Action& other) const {
    auto o = dynamic_cast<const DerivativeFilter*>(&other);
    return (o != nullptr);
}


void DerivativeFilter::print(std::ostream& out) const {
    out << "DerivativeFilter[method=" << method_ << "]";
}


void DerivativeFilter::execute(context::Context& ctx) const {
    auto timing(ctx.statistics().derivativeTimer());


    // Construct Nabla via FVM
    auto& field = ctx.field();
    auto grid   = field.representation()->atlasGrid();
    auto pts    = size_t(grid.size());

    atlas::MeshGenerator meshgenerator("structured");
    auto mesh = meshgenerator.generate(grid);

    atlas::numerics::fvm::Method fvm(mesh, atlas::option::levels(field.dimensions()));
    atlas::numerics::Nabla nabla(fvm);

    auto& nodes = fvm.node_columns().nodes();
    const atlas::mesh::IsGhostNode is_ghost(nodes);


    // Set input/output fields (copying is not great)
    auto scalar = fvm.node_columns().createField<double>(atlas::option::name("scalar"));
    auto grad   = fvm.node_columns().createField<double>(atlas::option::name("grad") | atlas::option::variables(2));

    auto vScalar = atlas::array::make_view<double, 2>(scalar);
    ASSERT(pts <= size_t(vScalar.shape(0)));

    for (size_t l = 0; l < field.dimensions(); ++l) {
        auto& values = field.direct(l);
        ASSERT(pts == values.size());

        size_t m = 0;
        for (atlas::idx_t n = 0; n < nodes.size(); ++n) {
            vScalar(n, l) = is_ghost(n) ? 0. : values[m++];
        }
        ASSERT(pts == m);
    }

    scalar.haloExchange();


    // Calculate gradients
    nabla.gradient(scalar, grad);


    // Set results
    auto vGrad = atlas::array::make_view<double, 3>(grad);

    for (size_t l = 0, lmax = field.dimensions(), d = 0; l < lmax; ++l) {
        for (auto ll : {atlas::LON, atlas::LAT}) {
            data::MIRValuesVector values(pts);

            size_t m = 0;
            for (atlas::idx_t n = 0; n < nodes.size(); ++n) {
                if (!is_ghost(n)) {
                    values[m++] = vGrad(n, l, ll);
                }
            }
            ASSERT(pts == m);

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
