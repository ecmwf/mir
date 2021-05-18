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


#include "mir/input/DummyInput.h"

#include <cmath>

#include "mir/data/MIRField.h"


namespace mir {
namespace input {


static ArtificialInputBuilder<DummyInput> __artificial("dummy");


static const param::SimpleParametrisation empty;


DummyInput::DummyInput(const param::MIRParametrisation& /*ignored*/) : ArtificialInput(empty) {
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


MIRValuesVector DummyInput::fill(size_t /*ignored*/) const {
    MIRValuesVector values(360 * 181, 42.);

    size_t k = 0;
    for (size_t i = 0; i < 360; ++i) {
        for (size_t j = 0; j < 181; ++j) {
            values[k++] = std::sin(double(i) / 10.) + std::cos(double(j) / 10.);
        }
    }

    return values;
}


}  // namespace input
}  // namespace mir
