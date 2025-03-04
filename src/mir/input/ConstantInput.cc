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
