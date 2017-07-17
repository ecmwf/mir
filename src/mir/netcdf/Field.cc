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

void Field::setMetadata(data::MIRField& mirField, size_t i) const {


    if (standardName_ == "air_pressure_at_sea_level") {
        mirField.metadata(i, "paramId", 167);
    }

    if (standardName_ == "air_temperature") {
        mirField.metadata(i, "paramId", 130);
    }

    if (standardName_ == "dew_point_temperature" ) {
        mirField.metadata(i, "paramId", 168);
    }

    if (standardName_ == "eastward_wind") {
        mirField.metadata(i, "paramId", 255);
    }

    if (standardName_ == "land_binary_mask") {
        mirField.metadata(i, "paramId", 255);
    }

    if (standardName_ == "lwe_thickness_of_large_scale_precipitation_amount") {
        mirField.metadata(i, "paramId", 255);
    }

    if (standardName_ == "moisture_content_of_soil_layer") {
        mirField.metadata(i, "paramId", 255);
    }

    if (standardName_ == "precipitation_amount") {
        mirField.metadata(i, "paramId", 255);
    }

    if (standardName_ == "precipitation_flux") {
        mirField.metadata(i, "paramId", 255);
    }

    if (standardName_ == "sea_ice_area_fraction") {
        mirField.metadata(i, "paramId", 255);
    }

    if (standardName_ == "sea_water_temperature") {
        mirField.metadata(i, "paramId", 255);
    }

    if (standardName_ == "snowfall_amount") {
        mirField.metadata(i, "paramId", 255);
    }

    if (standardName_ == "soil_temperature") {
        mirField.metadata(i, "paramId", 228139);
    }

    if (standardName_ == "surface_altitude") {
        mirField.metadata(i, "paramId", 255);
    }

    if (standardName_ == "surface_downward_eastward_stress") {
        mirField.metadata(i, "paramId", 255);
    }

    if (standardName_ == "surface_downward_northward_stress") {
        mirField.metadata(i, "paramId", 255);
    }

    if (standardName_ == "surface_upward_water_flux") {
        mirField.metadata(i, "paramId", 255);
    }

    if (standardName_ == "wind") {
        mirField.metadata(i, "paramId", 255);
    }

    if (standardName_ == "wind_speed") {
        mirField.metadata(i, "paramId", 255);
    }

    if (standardName_ == "x_wind") {
        mirField.metadata(i, "paramId", 131);
    }

    if (standardName_ == "y_wind") {
        mirField.metadata(i, "paramId", 132);
    }

}


}
}
