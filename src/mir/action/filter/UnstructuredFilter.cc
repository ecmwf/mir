// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/filter/UnstructuredFilter.h"

#include <memory>
#include <ostream>
#include <vector>

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/other/UnstructuredGrid.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Types.h"


namespace mir::action {


bool UnstructuredFilter::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const UnstructuredFilter*>(&other);
    return (o != nullptr);
}


void UnstructuredFilter::print(std::ostream& out) const {
    out << "UnstructuredFilter[]";
}


void UnstructuredFilter::execute(context::Context& ctx) const {
    auto& field = ctx.field();
    repres::RepresentationHandle repres(field.representation());

    size_t N = repres->numberOfPoints();
    ASSERT(N > 0);

    std::vector<double> latitudes(N);
    std::vector<double> longitudes(N);

    for (const std::unique_ptr<repres::Iterator> it(repres->iterator()); it->next();) {
        const auto& p              = **it;
        latitudes.at(it->index())  = p[0];
        longitudes.at(it->index()) = p[1];
    }

    field.representation(new repres::other::UnstructuredGrid(latitudes, longitudes));
}


const char* UnstructuredFilter::name() const {
    return "UnstructuredFilter";
}


static const ActionBuilder<UnstructuredFilter> __action("filter.unstructured");


}  // namespace mir::action
