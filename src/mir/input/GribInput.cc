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


#include "eckit/exception/Exceptions.h"
#include "eckit/io/BufferedHandle.h"

#include "mir/data/MIRField.h"
#include "mir/util/Grib.h"

#include "mir/input/GribInput.h"


namespace mir {
namespace input {
namespace {


static struct {
    const char *name;
    const char *key;
} mappings[] = {
    {"west_east_increment", "iDirectionIncrementInDegrees"},
    {"north_south_increment", "jDirectionIncrementInDegrees"},
    {"west", "longitudeOfFirstGridPointInDegrees"},
    {"east", "longitudeOfLastGridPointInDegrees"},
    {"north", "latitudeOfFirstGridPointInDegrees"},
    {"south", "latitudeOfLastGridPointInDegrees"},
    {"truncation", "pentagonalResolutionParameterJ"},// Assumes triangular truncation

    // This will be just called for has()
    {"gridded", "Nj"}, // Is that always true?
    {"spherical", "pentagonalResolutionParameterJ"},
    {0, 0},
};


static const char* get_key(const std::string& name) {
    const char *key = name.c_str();
    size_t i = 0;
    while (mappings[i].name) {
        if (name == mappings[i].name) {
            key = mappings[i].key;
            break;
        }
        i++;
    }
    return key;
}

}  // (anonymous namespace)




GribInput::GribInput() {
}


GribInput::~GribInput() {
}


const param::MIRParametrisation &GribInput::parametrisation() const {
    return *this;
}


data::MIRField *GribInput::field() const {
    ASSERT(grib_.get());

    size_t count;
    GRIB_CALL(grib_get_size(grib_.get(), "values", &count));

    size_t size = count;
    std::vector<double> values(count);
    GRIB_CALL(grib_get_double_array(grib_.get(), "values", &values[0], &size));
    ASSERT(count == size);

    long bitmap;
    GRIB_CALL(grib_get_long(grib_.get(), "bitmapPresent", &bitmap));

    double missing;
    GRIB_CALL(grib_get_double(grib_.get(), "missingValue", &missing));

    data::MIRField *field = new data::MIRField(bitmap != 0, missing);
    field->values(values);
    return field;
}


grib_handle *GribInput::gribHandle() const {
    return grib_.get();
}

bool GribInput::has(const std::string &name) const {
    const char *key = get_key(name);
    bool ok = grib_is_defined(grib_.get(), key);
    eckit::Log::info() << "GribInput::has(" << name << ",key=" << key << ") " << (ok ? "yes": "no") << std::endl;
    return ok;
}

bool GribInput::get(const std::string& name, bool& value) const {
    NOTIMP;
}

bool GribInput::get(const std::string& name, long& value) const {
    const char *key = get_key(name);
    int err = grib_get_long(grib_.get(), key, &value);

    if (err == GRIB_NOT_FOUND) {
        return FieldParametrisation::get(name, value);
    }

    if (err) {
        GRIB_ERROR(err, key);
    }

    eckit::Log::info() << "grib_get_long(" << name << ",key=" << key << ") " << value << std::endl;
    return true;
}

bool GribInput::get(const std::string& name, double& value) const {
    NOTIMP;
}

bool GribInput::get(const std::string& name, std::vector<long>& value) const {
    const char *key = get_key(name);

    size_t count = 0;
    int err = grib_get_size(grib_.get(), key, &count);

    if (err == GRIB_NOT_FOUND) {
        return FieldParametrisation::get(name, value);
    }

    if (err) {
        GRIB_ERROR(err, key);
    }

    size_t size = count;

    value.resize(count);

    GRIB_CALL(grib_get_long_array(grib_.get(), key, &value[0], &size));
    ASSERT(count == size);

    ASSERT(value.size());

    eckit::Log::info() << "grib_get_long_array(" << name << ",key=" << key << ") size=" << value.size() << std::endl;


    return true;
}

bool GribInput::get(const std::string& name, std::string& value) const {
    const char *key = get_key(name);

    char buffer[10240];
    size_t size = sizeof(buffer);
    int err = grib_get_string(grib_.get(), key, buffer, &size);

    if (err == GRIB_NOT_FOUND) {
        return FieldParametrisation::get(name, value);
    }

    if (err) {
        GRIB_ERROR(err, key);
    }

    ASSERT(size < sizeof(buffer) - 1);
    value = buffer;

    eckit::Log::info() << "grib_get_string(" << name << ",key=" << key << ") " << value << std::endl;

    return true;
}

bool GribInput::get(const std::string& name, std::vector<double>& value) const {
    return FieldParametrisation::get(name, value);
}

bool GribInput::handle(grib_handle *h) {
    grib_.reset(h);
    return h != 0;
}


}  // namespace input
}  // namespace mir

