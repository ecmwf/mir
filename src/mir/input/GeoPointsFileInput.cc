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

#include "mir/input/GeoPointsFileInput.h"
#include "mir/repres/other/UnstructuredGrid.h"

#include "eckit/parser/Tokenizer.h"
#include "eckit/utils/Translator.h"
#include "eckit/exception/Exceptions.h"

namespace mir {
namespace input {

// See https://software.ecmwf.int/wiki/display/METV/Geopoints

GeoPointsFileInput::GeoPointsFileInput(const std::string& path):
    path_(path) {

    parametrisation_.set("gridType", "unstructured_grid");
    parametrisation_.set("gridded", true);

    eckit::Tokenizer parse(" \t");
    eckit::Translator<std::string, double> s2d;

    std::ifstream in(path_);
    if (!in) {
        throw eckit::CantOpenFile(path_);
    }

    eckit::Tokenizer parse2("=");

    char line[10240];
    bool data = false;
    while (in.getline(line, sizeof(line))) {

        if (!data && strncmp(line, "# ", 2) == 0) {
            std::vector<std::string> v;
            // std::cout << "PARSE " << line +2 << std::endl;
            parse2(line + 2, v);
            ASSERT(v.size() == 2);
            parametrisation_.set(v[0], v[1]);
        }

        if (!data && strncmp(line, "#DATA", 5) == 0) {
            data = true;
            continue;
        }
        if (data) {
            std::vector<std::string> v;
            parse(line, v);
            if (v.size() >= 3) {
                latitudes_.push_back(s2d(v[0]));
                longitudes_.push_back(s2d(v[1]));
                values_.push_back(s2d(v.back()));
            }
        }
    }
}


GeoPointsFileInput::~GeoPointsFileInput() {}


bool GeoPointsFileInput::sameAs(const MIRInput& other) const {
    const GeoPointsFileInput* o = dynamic_cast<const GeoPointsFileInput*>(&other);
    return o && (path_ == o->path_);
}

bool GeoPointsFileInput::next() {
    return values_.size() != 0;
}


const param::MIRParametrisation &GeoPointsFileInput::parametrisation(size_t which) const {
    ASSERT(which == 0);
    return parametrisation_;
}


data::MIRField *GeoPointsFileInput::field() const {

    data::MIRField *field = new data::MIRField(new repres::other::UnstructuredGrid(latitudes_, longitudes_), false, 999.0);
    field->update(values_, 0);

    return field;
}


void GeoPointsFileInput::print(std::ostream &out) const {
    out << "GeoPointsFileInput[path=" << path_ << "]";
}

// From FieldParametrisation
void GeoPointsFileInput::latitudes(std::vector<double> &latitudes) const {
    latitudes = latitudes_;
}

void GeoPointsFileInput::longitudes(std::vector<double> &longitudes) const {
    longitudes = longitudes_;
}

const std::vector<double>& GeoPointsFileInput::latitudes() const {
    return latitudes_;
}

const std::vector<double>& GeoPointsFileInput::longitudes() const {
    return longitudes_;
}

const std::vector<double>& GeoPointsFileInput::values() const {
    return values_;
}



}  // namespace input
}  // namespace mir

