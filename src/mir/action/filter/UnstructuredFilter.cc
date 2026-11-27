// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/filter/UnstructuredFilter.h"

#include <memory>
#include <ostream>
#include <vector>

#include "eckit/geo/Grid.h"

#include "mir/action/context/Context.h"
#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/UnstructuredGrid.h"
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

    ASSERT(repres->numberOfPoints() > 0);

    const auto [latitudes, longitudes] = repres->canonicalGrid().to_latlons();
    field.representation(new repres::UnstructuredGrid(latitudes, longitudes));
}


const char* UnstructuredFilter::name() const {
    return "UnstructuredFilter";
}


static const ActionBuilder<UnstructuredFilter> __action("filter.unstructured");


}  // namespace mir::action
