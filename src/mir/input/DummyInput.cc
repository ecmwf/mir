/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "mir/input/DummyInput.h"

#include <cmath>
#include <iostream>

#include "mir/data/MIRField.h"


namespace mir {
namespace input {

static ArtificialInputBuilder<DummyInput> __artificial("dummy");


struct dummy_t : public param::SimpleParametrisation {
    dummy_t() {
        set("gridded", true);
        set("gridType", "regular_ll");
        set("north", 90.0);
        set("south", -90.0);
        set("west", 0.0);
        set("east", 359.0);
        set("west_east_increment", 1.0);
        set("south_north_increment", 1.0);
        set("Ni", 360);
        set("Nj", 181);
    }
} static const dummy;


DummyInput::DummyInput(const param::MIRParametrisation& /*ignored*/) : ArtificialInput(dummy) {}


bool DummyInput::sameAs(const MIRInput& other) const {
    auto o = dynamic_cast<const DummyInput*>(&other);
    return o && ArtificialInput::sameAs(other);
}


void DummyInput::print(std::ostream& out) const {
    out << "DummyInput[";
    ArtificialInput::print(out);
    out << "]";
}


data::MIRValuesVector DummyInput::fill(size_t /*ignored*/) const {
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
