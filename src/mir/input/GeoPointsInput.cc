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


#include <fstream>
#include <iostream>

#include "mir/data/MIRField.h"

#include "mir/input/GeoPointsInput.h"
#include "mir/repres/other/UnstructuredGrid.h"

#include "eckit/parser/Tokenizer.h"
#include "eckit/utils/Translator.h"
#include "eckit/exception/Exceptions.h"

namespace mir {
namespace input {

// See https://software.ecmwf.int/wiki/display/METV/Geopoints

GeoPointsInput::GeoPointsInput(const std::string& path):
    path_(path) {


    eckit::Tokenizer parse(" \t");
    eckit::Translator<std::string, double> s2d;

    std::ifstream in(path_);
    if(!in) {
        throw eckit::CantOpenFile(path_);
    }

    char line[10240];
    bool data = false;
    while(in.getline(line, sizeof(line))) {
        if(!data && strncmp(line, "#DATA", 5) == 0) {
            data = true;
            continue;
        }
        if(data) {
            std::vector<std::string> v;
            parse(line, v);
            if(v.size() >= 3) {
                latitudes_.push_back(s2d(v[0]));
                longitudes_.push_back(s2d(v[1]));
                values_.push_back(s2d(v.back()));
            }
        }
    }
 }


GeoPointsInput::~GeoPointsInput() {}


bool GeoPointsInput::sameAs(const MIRInput& other) const {
    const GeoPointsInput* o = dynamic_cast<const GeoPointsInput*>(&other);
    return o && (path_ == o->path_);
}

bool GeoPointsInput::next() {
    return values_.size() != 0;
}


const param::MIRParametrisation &GeoPointsInput::parametrisation() const {
    return *this;
}


data::MIRField *GeoPointsInput::field() const {

    data::MIRField *field = new data::MIRField(new repres::other::UnstructuredGrid(latitudes_, longitudes_), false, 999.0);
    field->update(values_, 0);

    return field;
}


void GeoPointsInput::print(std::ostream &out) const {
    out << "GeoPointsInput[path=" << path_ << "]";
}

bool GeoPointsInput::has(const std::string& name) const {
    if (name == "gridded") {
        return true;
    }
    if (name == "spectral") {
        return false;
    }
    return FieldParametrisation::has(name);
}

bool GeoPointsInput::get(const std::string &name, std::string &value) const {

    if (name == "gridType") {
        value = "unstructured_grid";
        return true;
    }
    return FieldParametrisation::get(name, value);
}

// From FieldParametrisation
void GeoPointsInput::latitudes(std::vector<double> &latitudes) const {
    latitudes = latitudes_;
}

void GeoPointsInput::longitudes(std::vector<double> &longitudes) const {
    longitudes = longitudes_;
}



}  // namespace input
}  // namespace mir

