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


namespace mir {
namespace input {


DummyInput::DummyInput() {
}


DummyInput::~DummyInput() {}


const param::MIRParametrisation &DummyInput::parametrisation() const {
    return *this;
}


data::MIRField *DummyInput::field() const {
    data::MIRField *field = new data::MIRField(false, 999.);

    std::vector< double > values(51 * 51, 42);
    size_t k = 0;
    for (size_t i = 0; i < 51; ++i)
        for (size_t j = 0; j < 51; ++j) {
            values[k++] = sin(double(i) / 10.) + cos(double(j) / 10.);
        }
    field->values(values, 0);

    return field;
}


void DummyInput::print(std::ostream &out) const {
    out << "DummyInput[...]";
}

bool DummyInput::has(const std::string& name) const {
    if(name == "gridded") {
        return true;
    }
    if(name == "spherical") {
        return false;
    }
    return FieldParametrisation::has(name);
}

bool DummyInput::get(const std::string &name, std::string &value) const {

    if (name == "gridType") {
        value = "regular_ll";
        return true;
    }
    return FieldParametrisation::get(name, value);
}

bool DummyInput::get(const std::string &name, double &value) const {

    if (name == "north") {
        value = 50;
        return true;
    }

    if (name == "south") {
        value = 0;
        return true;
    }

    if (name == "west") {
        value = 0;
        return true;
    }

    if (name == "east") {
        value = 50;
        return true;
    }

    if (name == "west_east_increment") {
        value = 1;
        return true;
    }

    if (name == "north_south_increment") {
        value = 1;
        return true;
    }

    return FieldParametrisation::get(name, value);
}



}  // namespace input
}  // namespace mir

