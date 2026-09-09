// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/input/DummyInput.h"

#include <cmath>

#include "mir/data/MIRField.h"
#include "mir/util/Exceptions.h"


namespace mir::input {


static const ArtificialInputBuilder<DummyInput> __artificial("dummy");


DummyInput::DummyInput() {
    parametrisation().set("gridded", true);
    parametrisation().set("gridType", "regular_ll");
    parametrisation().set("north", 90.0);
    parametrisation().set("south", -90.0);
    parametrisation().set("west", 0.0);
    parametrisation().set("east", 359.0);
    parametrisation().set("west_east_increment", 1.0);
    parametrisation().set("south_north_increment", 1.0);
    parametrisation().set("Ni", 360);
    parametrisation().set("Nj", 181);
}


data::MIRField DummyInput::field() const {
    ASSERT(dimensions() > 0);

    data::MIRField field(parametrisation(0), false, 9999.);
    MIRValuesVector values(360 * 181, 42.);

    size_t k = 0;
    for (size_t i = 0; i < 360; ++i) {
        for (size_t j = 0; j < 181; ++j) {
            values[k++] = std::sin(double(i) / 10.) + std::cos(double(j) / 10.);
        }
    }

    for (size_t which = 0; which < dimensions(); ++which) {
        field.update(values, which);
    }

    return field;
}


}  // namespace mir::input
