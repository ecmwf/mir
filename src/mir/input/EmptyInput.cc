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

#include "mir/input/EmptyInput.h"

#include "eckit/exception/Exceptions.h"

namespace mir {
namespace input {


EmptyInput::EmptyInput(): calls_(0) {
}


EmptyInput::~EmptyInput() {}


bool EmptyInput::sameAs(const MIRInput &other) const {
    const EmptyInput *o = dynamic_cast<const EmptyInput *>(&other);
    return o;
}

bool EmptyInput::next() {
    return calls_++ == 0;
}


const param::MIRParametrisation &EmptyInput::parametrisation(size_t which) const {
    ASSERT(which == 0);
    return *this;
}


data::MIRField *EmptyInput::field() const {
    data::MIRField *field = new data::MIRField(*this, false, 999.);
    return field;
}


void EmptyInput::print(std::ostream &out) const {
    out << "EmptyInput[...]";
}

bool EmptyInput::has(const std::string &name) const {
    if (name == "gridded") {
        return true;
    }
    if (name == "spectral") {
        return false;
    }
    return FieldParametrisation::has(name);
}

bool EmptyInput::get(const std::string &name, std::string &value) const {

    if (name == "gridType") {
        value = "unstructured_grid";
        return true;
    }
    return FieldParametrisation::get(name, value);
}

void EmptyInput::latitudes(std::vector<double> &) const {

}

void EmptyInput::longitudes(std::vector<double> &) const {

}


// bool EmptyInput::get(const std::string &name, double &value) const {

//     if (name == "north") {
//         value = 90;
//         return true;
//     }

//     if (name == "south") {
//         value = -90;
//         return true;
//     }

//     if (name == "west") {
//         value = 0;
//         return true;
//     }

//     if (name == "east") {
//         value = 359;
//         return true;
//     }

//     if (name == "west_east_increment") {
//         value = 1;
//         return true;
//     }

//     if (name == "south_north_increment") {
//         value = 1;
//         return true;
//     }

//     return FieldParametrisation::get(name, value);
// }



}  // namespace input
}  // namespace mir

