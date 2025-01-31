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


#include "mir/input/GridSpecInput.h"

#include <memory>

#include "eckit/geo/Grid.h"

#include "mir/data/MIRField.h"
#include "mir/param/GridSpecParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Types.h"


namespace mir::input {


static const ArtificialInputBuilder<GridSpecInput> __artificial("gridspec");


GridSpecInput::GridSpecInput(const param::MIRParametrisation& /*ignored*/) {
    parametrisation().set("gridded", true);
}


data::MIRField GridSpecInput::field() const {
    ASSERT(dimensions() > 0);

    std::string gridspec;
    parametrisation().get("gridspec", gridspec);

    param::GridSpecParametrisation param(eckit::geo::GridFactory::make_from_string(gridspec));
    data::MIRField field(repres::RepresentationFactory::build(param));

    auto n = param.grid().size();
    ASSERT(n > 0);

    MIRValuesVector values(n, 0.);

    for (size_t which = 0; which < dimensions(); ++which) {
        field.update(values, which);
    }

    return field;
}


}  // namespace mir::input
