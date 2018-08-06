/*
 * (C) Copyright 1996- ECMWF.
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
#include "eckit/serialisation/IfstreamStream.h"


namespace mir {
namespace input {

// See https://software.ecmwf.int/wiki/display/METV/Geopoints

GeoPointsFileInput::GeoPointsFileInput(const std::string& path, int which) :
    path_(path),
    which_(which),
    next_(0),
    hasMissing_(false),
    footprint_(0) {

    // For now, this should give an ovwerestimate of the memory footprint
    footprint_ = eckit::PathName(path).size();


    std::ifstream in(path_.c_str());
    if (!in) {
        throw eckit::CantOpenFile(path_);
    }

    char magic = in.peek();
    size_t count = 0;

    if (magic == '#') {
        count = readText(in);
    }
    else {
        count = readBinary(in);
    }

    if (count == 0) {
        std::ostringstream oss;
        oss << path_ << " is not a valid geopoints file";
        throw eckit::SeriousBug(oss.str());
    }

    if (which_ == -1 && count > 1) {
        std::ostringstream oss;
        oss << path_ << " is a multi-field geopoints file with " << count << " fields, please select which";
        throw eckit::SeriousBug(oss.str());
    }

    if (which_ >= count) {
        std::ostringstream oss;
        oss << path_ << " contains " << count << " fields, requested index is " << which_;
        throw eckit::SeriousBug(oss.str());
    }

    // set dimensions
    dimensions_ = size_t(count);
    ASSERT(dimensions_);

}

GeoPointsFileInput::~GeoPointsFileInput() = default;


size_t GeoPointsFileInput::readText(std::ifstream& in) {

    eckit::Tokenizer parse2("=");
    eckit::Tokenizer parse(" \t");
    eckit::Translator<std::string, double> s2d;

    char line[10240];
    bool data = false;
    int count = 0;

    enum { STANDARD = 0, XYV, XY_VECTOR, POLAR_VECTOR } format = STANDARD;

    while (in.getline(line, sizeof(line))) {

        if (strncmp(line, "#GEO", 4) == 0) {
            count++;

            if (which_ >= 0 && count  > which_ + 1) {
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

        if (!data && strncmp(line, "#FORMAT ", 8) == 0) {
            std::vector<std::string> v;
            parse(line + 8, v);
            ASSERT(v.size() == 1);

            format = v[0] == "XYV" ?          XYV :
                     v[0] == "XY_VECTOR" ?    XY_VECTOR :
                     v[0] == "POLAR_VECTOR" ? POLAR_VECTOR :
                     throw eckit::SeriousBug(path_ + " invalid format line '" + line + "'");
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
            if (which_ < 0) {
                data = true;
            }
            else {
                data = (count == which_ + 1);
            }
            continue;
        }

        if (data) {
            std::vector<std::string> v;
            parse(line, v);
            if (v.size() >= 3) {
                latitudes_  .push_back(s2d(v[format == XYV ? 1 : 0]));
                longitudes_ .push_back(s2d(v[format == XYV ? 0 : 1]));
                values_.push_back(s2d(v.back()));
            }
        }
    }

    return count;
}


size_t GeoPointsFileInput::readBinary(std::ifstream& in) {

    eckit::IfstreamStream s(in);
    size_t count = 0;

    eckit::Log::info() << "GeoPointsFileInput::readBinary " << path_ << std::endl;


    for (;;) {

        std::string what;
        s >> what;

        eckit::Log::info() << "GeoPointsFileInput::readBinary " << what << std::endl;

        if (what == "END") {
            break;
        }

        ASSERT(what == "GEO");

        count++;
        if (which_ >= 0 && count  > which_ + 1) {
            break;
        }

        fieldParametrisation_.reset();
        fieldParametrisation_.set("gridType", "unstructured_grid");
        fieldParametrisation_.set("gridded", true);

        latitudes_.clear();
        longitudes_.clear();
        values_.clear();

        std::string format;
        s >> format;
        ASSERT(format == "XYV");
        eckit::Log::info() << "GeoPointsFileInput::readBinary format=" << format << std::endl;

        size_t n = 0;
        s >> n;

        for (size_t i = 0; i < n; ++i) {
            std::string k, v;
            s >> k >> v;
            eckit::Log::info() << "GeoPointsFileInput::readBinary " << k << "=" << v << std::endl;

            fieldParametrisation_.set(k, v);
        }

        s >> n;
        eckit::Log::info() << "GeoPointsFileInput::readBinary " << n << " points " << std::endl;
        latitudes_.resize(n);
        longitudes_.resize(n);
        values_.resize(n);
        for (size_t i = 0; i < n; ++i) {
            s >> longitudes_[i]
              >> latitudes_[i]
              >> values_[i];

            eckit::Log::info() << "GeoPointsFileInput::readBinary points " << i << std::endl;

        }

    }

    return count;
}




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
    auto o = dynamic_cast<const GeoPointsFileInput*>(&other);
    return o && (path_ == o->path_);
}


bool GeoPointsFileInput::next() {
    return (next_++ < dimensions_);
}


size_t GeoPointsFileInput::dimensions() const {
    return dimensions_;
}


const param::MIRParametrisation &GeoPointsFileInput::parametrisation(size_t which) const {
    ASSERT(which == 0);
    return fieldParametrisation_;
}


data::MIRField GeoPointsFileInput::field() const {

    data::MIRField field(new repres::other::UnstructuredGrid(latitudes_, longitudes_), hasMissing_, missingValue_);

    // copy, to preserve consistent internal state
    MIRValuesVector values(values_);
    field.update(values, 0);

    return field;
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


const MIRValuesVector& GeoPointsFileInput::values() const {
    return values_;
}


size_t GeoPointsFileInput::footprint() const {
    return footprint_;
}


static MIRInputBuilder< GeoPointsFileInput > __mirinput(0x2347454f); // "#GEO"


}  // namespace input
}  // namespace mir

