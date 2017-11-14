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


#include "mir/input/GeoPointsFileInput.h"

#include <fstream>
#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "eckit/log/Plural.h"
#include "eckit/parser/Tokenizer.h"
#include "eckit/utils/Translator.h"
#include "mir/data/MIRField.h"
#include "mir/repres/other/UnstructuredGrid.h"
#include "mir/util/GlobaliseUnstructured.h"


namespace mir {
namespace input {

// See https://software.ecmwf.int/wiki/display/METV/Geopoints

GeoPointsFileInput::GeoPointsFileInput(const std::string& path, int which) :
    path_(path),
    which_(which),
    hasMissing_(false) {

    eckit::Tokenizer parse(" \t");
    eckit::Translator<std::string, double> s2d;

    std::ifstream in(path_.c_str());
    if (!in) {
        throw eckit::CantOpenFile(path_);
    }

    eckit::Tokenizer parse2("=");

    char line[10240];
    bool data = false;
    int count = 0;

    while (in.getline(line, sizeof(line))) {

        if (strncmp(line, "#GEO", 4) == 0) {
            count++;

            if (which >= 0 && count  > which + 1) {
                break;
            }

            fieldParametrisation_.reset();
            fieldParametrisation_.set("gridType", "unstructured_grid");
            fieldParametrisation_.set("gridded", true);

            data = false;
            latitudes_.clear();
            longitudes_.clear();
            values_.clear();
            continue;
        }

        if (!data && strncmp(line, "# ", 2) == 0) {
            std::vector<std::string> v;
            // eckit::Log::info() << "PARSE " << line +2 << std::endl;
            parse2(line + 2, v);
            ASSERT(v.size() == 2);
            fieldParametrisation_.set(v[0], v[1]);

            // eckit::Log::info() << path_ << " ===> " << v[0] << "=" << v[1] << std::endl;
        }

        if (!data && strncmp(line, "#DATA", 5) == 0) {
            if (which < 0) {
                data = true;
            }
            else {
                data = (count == which + 1);
            }
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

    if (count == 0) {
        std::ostringstream oss;
        oss << path_ << " is not a valid geopoints file";
        throw eckit::SeriousBug(oss.str());
    }

    if (which == -1 && count > 1) {
        std::ostringstream oss;
        oss << path_ << " is a multi-field geopoints file with " << count << " fields, please select which";
        throw eckit::SeriousBug(oss.str());
    }

    if (which >= count) {
        std::ostringstream oss;
        oss << path_ << " contains " << count << " fields, requested index is " << which;
        throw eckit::SeriousBug(oss.str());
    }

    // set dimensions
    dimensions_ = size_t(count);
    ASSERT(dimensions_);
#if 0

    // reset missing values to something acceptable
    hasMissing_ = resetMissingValue(missingValue_);
    // globalise, appending missing values if necessary
    util::GlobaliseUnstructured globalise(parametrisation_);
    size_t nbExtraValues = globalise.appendGlobalPoints(latitudes_, longitudes_);
    if (nbExtraValues) {
        eckit::Log::info() << "GeoPointsFileInput: appending " << eckit::Plural(nbExtraValues, "missing value") << std::endl;

        ASSERT(latitudes_.size() == values_.size() + nbExtraValues);
        ASSERT(latitudes_.size() == longitudes_.size());
        values_.insert(values_.end(), nbExtraValues, missingValue_);
    }
#endif
}


GeoPointsFileInput::~GeoPointsFileInput() {}


bool GeoPointsFileInput::resetMissingValue(double& missingValue) {

    // geopoints hard-coded value, all values have to be below
    missingValue = 3e38;
    bool hasMissing = (values_.end() != std::find(values_.begin(), values_.end(), missingValue));

    // find the non-missing max value
    bool allMissing = true;
    double max = missingValue;
    for (const double& v : values_) {
        ASSERT(v <= missingValue);
        if (v != missingValue) {
            allMissing = false;
            if (max == missingValue || max < v)
                max = v;
        }
    }

    // if all values are missing set an acceptable value, otherwise use max + 1
    if (allMissing) {
        missingValue = 999.;
        values_.assign(values_.size(), missingValue);
        return true;
    }

    const double tempMissingValue = max + 1.;
    ASSERT(tempMissingValue == tempMissingValue);

    if (hasMissing) {
        for (double& v : values_) {
            if (v == missingValue)
                v = tempMissingValue;
        }
    }

    missingValue = tempMissingValue;
    ASSERT(missingValue_ < 3e38);

    return hasMissing;
}


bool GeoPointsFileInput::sameAs(const MIRInput& other) const {
    const GeoPointsFileInput* o = dynamic_cast<const GeoPointsFileInput*>(&other);
    return o && (path_ == o->path_);
}


bool GeoPointsFileInput::next() {
    return values_.size() != 0;
}


size_t GeoPointsFileInput::dimensions() const {
    return dimensions_;
}


const param::MIRParametrisation &GeoPointsFileInput::parametrisation(size_t which) const {
    ASSERT(which == 0);
    return fieldParametrisation_;
}


data::MIRField GeoPointsFileInput::field() const {

    NOTIMP;
    // data::MIRField field(new repres::other::UnstructuredGrid(latitudes_, longitudes_), hasMissing_, missingValue_);
    // field.update(values_, 0);

    // return field;
}


void GeoPointsFileInput::print(std::ostream &out) const {
    out << "GeoPointsFileInput["
        "path=" << path_
        << ",which=" << which_
        << ",dimensions=" << dimensions_
        << "]";
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

