/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/input/GeoPointsFileInput.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <fstream>
#include <ostream>
#include <sstream>

#include "eckit/filesystem/PathName.h"
#include "eckit/serialisation/IfstreamStream.h"
#include "eckit/utils/Tokenizer.h"
#include "eckit/utils/Translator.h"

#include "mir/data/MIRField.h"
#include "mir/repres/other/UnstructuredGrid.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace input {


static constexpr double missingValueGeoPoints = 3e38;
static constexpr double missingValueDefault   = 999.;


// See https://software.ecmwf.int/wiki/display/METV/Geopoints
GeoPointsFileInput::GeoPointsFileInput(const std::string& path, int which) :
    path_(path),
    next_(0),
    footprint_(size_t(eckit::PathName(path).size())),
    missingValue_(missingValueGeoPoints),
    which_(which),
    hasMissing_(false) {  // For now, this should overestimate the memory footprint

    std::ifstream in(path_.c_str());
    if (!in) {
        throw exception::CantOpenFile(path_);
    }

    auto magic   = char(in.peek());
    size_t count = (magic == '#' ? readText(in) : readBinary(in));

    if (count == 0) {
        std::ostringstream oss;
        oss << path_ << " is not a valid geopoints file";
        throw exception::SeriousBug(oss.str());
    }

    if (which_ == -1 && count > 1) {
        std::ostringstream oss;
        oss << path_ << " is a multi-field geopoints file with " << count << " fields, please select which";
        throw exception::SeriousBug(oss.str());
    }

    if (which_ >= int(count)) {
        std::ostringstream oss;
        oss << path_ << " contains " << count << " fields, requested index is " << which_;
        throw exception::SeriousBug(oss.str());
    }

    // set dimensions
    dimensions_ = size_t(count);
    ASSERT(dimensions_);


    repres::other::UnstructuredGrid::check(std::string("GeoPointsFileInput from ") + path, latitudes_, longitudes_);
}


GeoPointsFileInput::~GeoPointsFileInput() = default;


size_t GeoPointsFileInput::readText(std::ifstream& in) {
    constexpr size_t lenGEO        = 4;
    constexpr size_t lenFORMAT     = 8;
    constexpr size_t lenCOLUMNS    = 8;
    constexpr size_t lenDATA       = 5;
    constexpr size_t lenLineBuffer = 10240;

    auto sane = [](const std::string& s) {
        auto t(s);
        std::transform(t.begin(), t.end(), t.begin(), [=](std::string::value_type c) { return std::tolower(c); });
        return t;
    };

    eckit::Tokenizer parse_equals("=");
    eckit::Tokenizer parse(" \t");
    eckit::Translator<std::string, double> s2d;

    char line[lenLineBuffer];
    bool data = false;
    int count = 0;

    size_t latIndex = 0;
    size_t lonIndex = 1;

    while (in.getline(line, sizeof(line))) {

        if (std::strncmp(line, "#GEO", lenGEO) == 0) {
            count++;

            if (which_ >= 0 && count > which_ + 1) {
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

        if (!data && std::strncmp(line, "#FORMAT ", lenFORMAT) == 0) {
            std::vector<std::string> v;
            parse(line + lenFORMAT, v);
            ASSERT(v.size() == 1);

            if (v[0] == "XYV") {
                lonIndex = 0;
                latIndex = 1;
                continue;
            }
        }

        if (!data && std::strncmp(line, "#COLUMNS", lenCOLUMNS) == 0) {
            ASSERT(in.getline(line, sizeof(line)));

            std::vector<std::string> v;
            parse(line, v);
            ASSERT(v.size() >= 3);

            latIndex = 0;
            lonIndex = 0;
            for (size_t i = 0; i < v.size(); ++i) {
                auto s = sane(v[i]);

                if (s == "lon" || s == "x/long" || s == "long" || s == "longitude") {
                    lonIndex = i;
                    continue;
                }
                if (s == "lat" || s == "y/lat" || s == "latitude") {
                    latIndex = i;
                    continue;
                }
            }

            ASSERT(latIndex != lonIndex);
            continue;
        }

        if (!data && std::strncmp(line, "#DATA", lenDATA) == 0) {
            data = (which_ < 0) || (count == which_ + 1);
            continue;
        }

        if (!data) {
            std::vector<std::string> v;
            parse_equals(line + (std::strncmp(line, "#", 1) == 0 ? 1 : 0), v);

            if (v.size() == 2) {
                auto key = sane(v[0]) == "parameter" ? "paramId" : v[0];

                fieldParametrisation_.set(key, v[1]);
                continue;
            }
        }

        if (data) {
            std::vector<std::string> v;
            parse(line, v);
            if (v.size() >= 3) {
                latitudes_.push_back(s2d(v[latIndex]));
                longitudes_.push_back(s2d(v[lonIndex]));
                values_.push_back(s2d(v.back()));
            }
        }
    }

    return size_t(count);
}


size_t GeoPointsFileInput::readBinary(std::ifstream& in) {

    eckit::IfstreamStream s(in);
    size_t count = 0;

    for (;;) {

        std::string what;
        s >> what;

        if (what == "END") {
            break;
        }

        ASSERT(what == "GEO");

        count++;
        if (which_ >= 0 && int(count) > which_ + 1) {
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

        for (;;) {
            std::string key;
            std::string value;
            s >> key;
            if (key == "-") {
                break;
            }
            s >> value;

            fieldParametrisation_.set(key, value);
        }

        size_t n;
        s >> n;
        latitudes_.resize(n);
        longitudes_.resize(n);
        values_.resize(n);
        for (size_t i = 0; i < n; ++i) {
            s >> longitudes_[i] >> latitudes_[i] >> values_[i];
        }
    }


    return count;
}


bool GeoPointsFileInput::resetMissingValue(double& missingValue) {

    // geopoints hard-coded value, all values have to be below
    missingValue    = missingValueGeoPoints;
    bool hasMissing = (values_.end() != std::find(values_.begin(), values_.end(), missingValue));

    // find the non-missing max value
    bool allMissing = true;
    double max      = missingValue;
    for (const double& v : values_) {
        ASSERT(v <= missingValue);
        if (v != missingValue) {
            allMissing = false;
            if (max == missingValue || max < v) {
                max = v;
            }
        }
    }

    // if all values are missing set an acceptable value, otherwise use max + 1
    if (allMissing) {
        missingValue = missingValueDefault;
        values_.assign(values_.size(), missingValue);
        return true;
    }

    const double tempMissingValue = max + 1.;
    ASSERT(tempMissingValue == tempMissingValue);

    if (hasMissing) {
        for (double& v : values_) {
            if (v == missingValue) {
                v = tempMissingValue;
            }
        }
    }

    missingValue = tempMissingValue;
    ASSERT(missingValue_ < missingValueGeoPoints);

    return hasMissing;
}


bool GeoPointsFileInput::sameAs(const MIRInput& other) const {
    auto o = dynamic_cast<const GeoPointsFileInput*>(&other);
    return (o != nullptr) && (path_ == o->path_);
}


bool GeoPointsFileInput::next() {
    return (next_++ < dimensions_);
}


size_t GeoPointsFileInput::dimensions() const {
    return dimensions_;
}


const param::MIRParametrisation& GeoPointsFileInput::parametrisation(size_t which) const {
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


void GeoPointsFileInput::print(std::ostream& out) const {
    out << "GeoPointsFileInput["
           "path="
        << path_ << ",which=" << which_ << ",dimensions=" << dimensions_ << "]";
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


static MIRInputBuilder<GeoPointsFileInput> __mirinput(0x2347454f);  // "#GEO"


}  // namespace input
}  // namespace mir
