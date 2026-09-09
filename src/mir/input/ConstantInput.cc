// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/input/ConstantInput.h"

#include "mir/data/MIRField.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Types.h"


namespace mir::input {


static const ArtificialInputBuilder<ConstantInput> __artificial("constant");


data::MIRField ConstantInput::field() const {
    ASSERT(dimensions() > 0);

    double constant = 0;
    parametrisation().get("constant", constant);

    data::MIRField field(parametrisation(0), false, 9999.);

    repres::RepresentationHandle repres(field.representation());
    auto n = repres->numberOfValues();
    ASSERT(n > 0);

    MIRValuesVector values(n, constant);

    for (size_t which = 0; which < dimensions(); ++which) {
        field.update(values, which);
    }

    return field;
}


}  // namespace mir::input
