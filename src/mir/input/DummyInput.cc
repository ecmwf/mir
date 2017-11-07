/*
 * (C) Copyright 1996-2015 ECMWF.
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


#include <cmath>
#include <iostream>

#include "mir/data/MIRField.h"

#include "mir/input/DummyInput.h"
#include "eckit/exception/Exceptions.h"


namespace mir {
namespace input {


DummyInput::DummyInput(): calls_(0) {
    parametrisation_.set("gridded", true);
    parametrisation_.set("gridType", "regular_ll");
    parametrisation_.set("north", 90.0);
    parametrisation_.set("south", -90.0);
    parametrisation_.set("west", 0.0);
    parametrisation_.set("east", 359.0);
    parametrisation_.set("west_east_increment", 1.0);
    parametrisation_.set("south_north_increment", 1.0);
}

DummyInput::~DummyInput() {}


bool DummyInput::sameAs(const MIRInput& other) const {
    const DummyInput* o = dynamic_cast<const DummyInput*>(&other);
    return o;
}

bool DummyInput::next() {
    return calls_++ == 0;
}


const param::MIRParametrisation &DummyInput::parametrisation(size_t which) const {
    ASSERT(which == 0);
    return parametrisation_;
}


data::MIRField DummyInput::accessField() const {
    data::MIRField field(parametrisation_, false, 999.);

    std::vector< double > values(360 * 181, 42);
    size_t k = 0;
    for (size_t i = 0; i < 360; ++i)
        for (size_t j = 0; j < 181; ++j) {
            values[k++] = sin(double(i) / 10.) + cos(double(j) / 10.);
        }
    field.update(values, 0);

    return field;
}


void DummyInput::print(std::ostream &out) const {
    out << "DummyInput[...]";
}


}  // namespace input
}  // namespace mir

