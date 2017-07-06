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

#include "mir/input/NetcdfFileInput.h"

#include "eckit/exception/Exceptions.h"

#include "mir/api/mir_config.h"
#include "mir/data/MIRField.h"
#include "metkit/netcdf/Field.h"


namespace mir {
namespace input {


NetcdfFileInput::NetcdfFileInput(const eckit::PathName &path):
    path_(path),
    cache_(*this),
    dataset_(path, *this),
    fields_(dataset_.fields()),
    current_(-1) {


    for (auto j  = fields_.begin(); j != fields_.end(); ++j ) {
        std::cout << "NC " << *(*j) << std::endl;
    }

}


NetcdfFileInput::~NetcdfFileInput() {
    for (auto j = fields_.begin(); j != fields_.end(); ++j) {
        delete (*j);
    }
}


void NetcdfFileInput::print(std::ostream &out) const {
    out << "NetcdfFileInput[path=" << path_ << "]";
}


const param::MIRParametrisation &NetcdfFileInput::parametrisation(size_t which) const {
    ASSERT(which == 0);
    return cache_;
}


bool NetcdfFileInput::next() {
    cache_.reset();
    current_++;

    if (current_ >= fields_.size()) {
        return false;
    }


    return true;
}



data::MIRField NetcdfFileInput::field() const {
#if 0
    std::vector<double> values;
    getVariable(variable_, values);

    bool hasMissing = false; // Should check!
    double missingValue = 9999; // Read from file

    data::MIRField field(*this, hasMissing, missingValue);
    field.update(values, 0);

    return field;
#endif
}

bool NetcdfFileInput::get(const std::string& name, long& value) const {
    if (name == "paramId") {
        value = 1;
        return true;
    }
    return FieldParametrisation::get(name, value);
}



bool NetcdfFileInput::has(const std::string& name) const {
    if (name == "gridded") {
        return true;
    }
    if (name == "spectral") {
        return false;
    }
    return FieldParametrisation::has(name);
}

bool NetcdfFileInput::get(const std::string &name, std::string &value) const {

    if (name == "grid") {
        value = "regular_ll";
        return true;
    }
    return FieldParametrisation::get(name, value);
}

bool NetcdfFileInput::get(const std::string &name, double &value) const {


    if (name == "north") {
        value = latitude_[0];
        return true;
    }

    if (name == "south") {
        value = latitude_[latitude_.size() - 1];
        return true;
    }

    if (name == "west") {
        value = longitude_[0];
        return true;
    }

    if (name == "east") {
        value = longitude_[longitude_.size() - 1];
        return true;
    }

    if (name == "west_east_increment") {
        value = longitude_[1] - longitude_[0];
        return true;
    }

    if (name == "south_north_increment") {
        value = latitude_[0] - latitude_[1];
        return true;
    }


    return FieldParametrisation::get(name, value);
}

bool NetcdfFileInput::sameAs(const MIRInput& other) const {
    const NetcdfFileInput* o = dynamic_cast<const NetcdfFileInput*>(&other);
    return o && (path_ == o->path_);
}


static MIRInputBuilder< NetcdfFileInput > netcdf4(0x89484446); // ".HDF"
static MIRInputBuilder< NetcdfFileInput > netcdf3(0x43444601); // "CDF."



}  // namespace input
}  // namespace mir

