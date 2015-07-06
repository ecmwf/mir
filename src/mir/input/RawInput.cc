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

#include "mir/input/RawInput.h"
#include "eckit/exception/Exceptions.h"


namespace mir {
namespace input {


RawInput::RawInput(param::MIRParametrisation& in, const double* values, size_t count):
values_(values),
count_(count) {
}


RawInput::~RawInput() {}

bool RawInput::next() {
    NOTIMP;
}


const param::MIRParametrisation &RawInput::parametrisation() const {
    return *this;
}


data::MIRField *RawInput::field() const {
    data::MIRField *field = new data::MIRField(*this, false, 999.);

    std::vector< double > values(count_);
    ::memcpy(&values[0], values_, sizeof(double) * count_);
    field->values(values, 0);

    return field;
}


void RawInput::print(std::ostream &out) const {
    out << "RawInput[count=" << count_ << "]";
}

bool RawInput::has(const std::string& name) const {
    if(name == "gridded") {
        return true;
    }
    if(name == "spectral") {
        return false;
    }
    return FieldParametrisation::has(name);
}

bool RawInput::get(const std::string &name, std::string &value) const {

    if (name == "gridType") {
        value = "regular_ll";
        return true;
    }
    return FieldParametrisation::get(name, value);
}

bool RawInput::get(const std::string &name, double &value) const {

    if (name == "north") {
        value = 90;
        return true;
    }

    if (name == "south") {
        value = -90;
        return true;
    }

    if (name == "west") {
        value = 0;
        return true;
    }

    if (name == "east") {
        value = 359;
        return true;
    }

    if (name == "west_east_increment") {
        value = 1;
        return true;
    }

    if (name == "south_north_increment") {
        value = 1;
        return true;
    }

    return FieldParametrisation::get(name, value);
}



}  // namespace input
}  // namespace mir

