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
#include "metkit/netcdf/HyperCube.h"


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
    ASSERT(current_ >= 0 && current_ < fields_.size());

    std::vector<double> values;
    fields_[current_]->values(values);

    bool hasMissing = false; // Should check!
    double missingValue = 9999; // Read from file

    data::MIRField field(*this, hasMissing, missingValue);
    field.update(values, 0);

    return field;
}

bool NetcdfFileInput::get(const std::string& name, long& value) const {
    if (name == "paramId") {
        ASSERT(current_ >= 0 && current_ < fields_.size());
        value = fields_[current_]->paramId();
        return true;
    }


    if (name == "Ni") {
        ASSERT(current_ >= 0 && current_ < fields_.size());
        auto d = fields_[current_]->dimensions();
        ASSERT(d.size() >= 2);
        value = d[d.size() - 1];
        std::cout << "===== Ni " << value << std::endl;

        return true;
    }


    if (name == "Nj") {
        ASSERT(current_ >= 0 && current_ < fields_.size());
        auto d = fields_[current_]->dimensions();
        ASSERT(d.size() >= 2);
        value = d[d.size() - 2];
        std::cout << "===== Nj " << value << std::endl;
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

    if (name == "gridType") {
        ASSERT(current_ >= 0 && current_ < fields_.size());
        value = fields_[current_]->gridType();
        return true;
    }
    return FieldParametrisation::get(name, value);
}

bool NetcdfFileInput::get(const std::string &name, double &value) const {

    ASSERT(current_ >= 0 && current_ < fields_.size());

    if (name == "north") {
        value = fields_[current_]->north();
        return true;
    }

    if (name == "south") {
        value = fields_[current_]->south();
        return true;
    }

    if (name == "west") {
        value = fields_[current_]->west();
        return true;
    }

    if (name == "east") {
        value = fields_[current_]->east();
        return true;
    }

    if (name == "west_east_increment") {
        value = fields_[current_]->westEastIncrement();
        return true;
    }

    if (name == "south_north_increment") {
        value = fields_[current_]->southNorthIncrement();
        return true;
    }


    return FieldParametrisation::get(name, value);
}

bool NetcdfFileInput::sameAs(const MIRInput& other) const {
    const NetcdfFileInput* o = dynamic_cast<const NetcdfFileInput*>(&other);
    return o && (path_ == o->path_);
}

size_t NetcdfFileInput::dimensions() const {
    ASSERT(current_ >= 0 && current_ < fields_.size());
    std::vector<size_t> dims = fields_[current_]->dimensions();

    std::cout << "NC dimensions: " << dims << std::endl;

    ASSERT(dims.size() >= 2);
    // Assumes lat/lon at the end

    size_t n = 1;
    for (size_t i = 0; i < dims.size() - 2; ++i) {
        n *= dims[i];
    }


    std::cout << "NC dimensions: " << n << std::endl;

    return n;
}


static MIRInputBuilder< NetcdfFileInput > netcdf4(0x89484446); // ".HDF"
static MIRInputBuilder< NetcdfFileInput > netcdf3(0x43444601); // "CDF."



}  // namespace input
}  // namespace mir

