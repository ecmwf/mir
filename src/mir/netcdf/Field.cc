/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// Baudouin Raoult - ECMWF Jan 2015

#include "mir/netcdf/Field.h"

#include "mir/netcdf/GridSpec.h"
#include "mir/netcdf/Variable.h"
#include "mir/data/MIRField.h"
#include "eckit/parser/YAMLParser.h"

#include <iostream>

namespace mir {
namespace netcdf {

Field::Field(const Variable &variable):
    variable_(variable),
    standardName_(variable.getAttributeValue<std::string>("standard_name")),
    units_(variable.getAttributeValue<std::string>("units")) {
}

Field::~Field() {
}

const GridSpec &Field::gridSpec() const {
    if (!gridSpec_) {
        // TODO: may need a mutex
        gridSpec_.reset(GridSpec::create(variable_));
        std::cout << *gridSpec_ << std::endl;
    }
    return *gridSpec_;
}
// ==========================================================

void Field::get2DValues(std::vector<double>& values, size_t i) const {
    variable_.get2DValues(values, i);
    gridSpec().reorder(values);
}

size_t Field::count2DValues() const {
    return variable_.count2DValues();
}

bool Field::hasMissing() const {
    return variable_.hasMissing();
}

double Field::missingValue() const {
    return variable_.missingValue();
}

// ==========================================================

bool Field::has(const std::string& name) const {
    return gridSpec().has(name);
}

bool Field::get(const std::string&name, long& value) const {
    if (name == "paramId") {
        value = 255;
        return true;
    }
    return gridSpec().get(name, value);
}

bool Field::get(const std::string&name, std::string& value) const {
    return gridSpec().get(name, value);
}

bool Field::get(const std::string &name, double &value) const {
    return gridSpec().get(name, value);
}


bool Field::get(const std::string &name, std::vector<double> &value) const {
    return gridSpec().get(name, value);
}

void Field::print(std::ostream &out) const {
    out << "Field[variable=" << variable_ << "]";
}

static pthread_once_t once = PTHREAD_ONCE_INIT;

static eckit::Value standard_names;


static void init() {
    standard_names = eckit::YAMLParser::decodeFile("~mir/etc/mir/netcdf.yaml");
    standard_names.dump(std::cout) << std::endl;
}

void Field::setMetadata(data::MIRField& mirField, size_t i) const {

    pthread_once(&once, init);

    eckit::Value s = standard_names[standardName_];

    if (s.isMap()) {
        eckit::ValueMap m = s;
        for (auto k : m) {
            mirField.metadata(i, k.first, k.second);
        }

    }
    else {
        eckit::Log::warning()
                << "No mapping for NetCDF standard name ["
                << standardName_
                << "] "
                << variable_
                << std::endl;
    }
}


}
}
