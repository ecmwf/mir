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

#include "soyuz/data/MIRField.h"

#include "soyuz/input/DummyInput.h"


namespace mir {
namespace input {


DummyInput::DummyInput() {
    settings_["gridType"] = "regular_ll";
    settings_["north"] = "50";
    settings_[ "west"] = "0";
    settings_["south"] = "0";
    settings_["east"] = "50";
    settings_["north_south_increment"] = "1";
    settings_["west_east_increment"] = "1";
}


DummyInput::~DummyInput() {}


const MIRParametrisation &DummyInput::parametrisation() const {
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
    field->values(values);

    return field;
}


void DummyInput::print(std::ostream &out) const {
    out << "DummyInput[...]";
}


bool DummyInput::lowLevelGet(const std::string &name, std::string &value) const {
    std::map<std::string, std::string>::const_iterator j = settings_.find(name);
    if (j != settings_.end()) {
        value = (*j).second;
        return true;
    }
    return false;
}


}  // namespace input
}  // namespace mir

