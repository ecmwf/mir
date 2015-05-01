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
    grib_values values;
} mappings[] = {
    {"west_east_increment", "iDirectionIncrementInDegrees"},
    {"north_south_increment", "jDirectionIncrementInDegrees"},
    {"west", "longitudeOfFirstGridPointInDegrees"},
    {"east", "longitudeOfLastGridPointInDegrees"},
    {"north", "latitudeOfFirstGridPointInDegrees"},
    {"south", "latitudeOfLastGridPointInDegrees"},
    {"truncation", "pentagonalResolutionParameterJ", {0}},// Assumes triangular truncation

    // This will be just called for has()
    {"gridded", "numberOfPointsAlongAMeridian"}, // Is that always true?
    {"spherical", "pentagonalResolutionParameterJ", {0}},

    /// FIXME: Find something that does no clash
    {"reduced", "numberOfParallelsBetweenAPoleAndTheEquator", {"isOctahedral", GRIB_TYPE_LONG, 0 }},
    {"regular", "N", {"gridType", GRIB_TYPE_STRING, 0, 0.0, "regular_gg"}},
    {"octahedral", "numberOfParallelsBetweenAPoleAndTheEquator", {"isOctahedral", GRIB_TYPE_LONG, 1 }},


    {0, 0},
};


static const char *get_key(const std::string &name, grib_values *&values) {
    const char *key = name.c_str();
    size_t i = 0;
    while (mappings[i].name) {
        if (name == mappings[i].name) {
            key = mappings[i].key;
            values = &mappings[i].values;
            break;
        }
        i++;
    }
    return key;
}

static const char *get_key(const std::string &name) {
    grib_values *ignore;
    return get_key(name, ignore);
}

}  // (anonymous namespace)




GribInput::GribInput(): grib_(0) {
}


GribInput::~GribInput() {
    handle(0); // Will delete handle
}


const param::MIRParametrisation &GribInput::parametrisation() const {
    return *this;
}


data::MIRField *GribInput::field() const {
    ASSERT(grib_);

    size_t count;
    GRIB_CALL(grib_get_size(grib_, "values", &count));

    size_t size = count;
    std::vector<double> values(count);
    GRIB_CALL(grib_get_double_array(grib_, "values", &values[0], &size));
    ASSERT(count == size);

    long bitmap;
    GRIB_CALL(grib_get_long(grib_, "bitmapPresent", &bitmap));

    double missing;
    GRIB_CALL(grib_get_double(grib_, "missingValue", &missing));

    data::MIRField *field = new data::MIRField(bitmap != 0, missing);
    field->values(values, 0);
    return field;
}


grib_handle *GribInput::gribHandle() const {
    return grib_;
}

bool GribInput::has(const std::string &name) const {
    grib_values *values = 0;
    const char *key = get_key(name, values);

    eckit::Log::info() << "GribInput::has(" << name << ") " << values << std::endl;

    bool ok = false;

    if (values->name) {

        switch (values->type) {
        case GRIB_TYPE_LONG: {
            long value;
            if (GribInput::get(values->name, value)) {
                ok = value == values->long_value;
            }
        }
        break;

        case GRIB_TYPE_DOUBLE: {
            double value;
            if (GribInput::get(values->name, value)) {
                ok = value == values->double_value;
            }
        }
        break;

        case GRIB_TYPE_STRING: {
            std::string value;
            if (GribInput::get(values->name, value)) {
                ok = value == values->string_value;
            }
        }
        break;

        default:
            ASSERT(false);
        }

    } else {
        ok = grib_is_defined(grib_, key);
    }
    eckit::Log::info() << "GribInput::has(" << name << ",key=" << key << ") " << (ok ? "yes" : "no") << std::endl;
    return ok;
}

bool GribInput::get(const std::string &name, bool &value) const {
    long temp;
    const char *key = get_key(name);
    int err = grib_get_long(grib_, key, &temp);

    if (err == GRIB_NOT_FOUND) {
        return FieldParametrisation::get(name, value);
    }

    if (err) {
        eckit::Log::info() << "grib_get_bool(" << name << ",key=" << key << ") failed " << err << std::endl;
        GRIB_ERROR(err, key);
    }

    value = temp != 0;

    eckit::Log::info() << "grib_get_bool(" << name << ",key=" << key << ") " << value << std::endl;
    return true;
}

bool GribInput::get(const std::string &name, long &value) const {
    const char *key = get_key(name);
    int err = grib_get_long(grib_, key, &value);

    if (err == GRIB_NOT_FOUND) {
        return FieldParametrisation::get(name, value);
    }

    if (err) {
        eckit::Log::info() << "grib_get_long(" << name << ",key=" << key << ") failed " << err << std::endl;
        GRIB_ERROR(err, key);
    }

    eckit::Log::info() << "grib_get_long(" << name << ",key=" << key << ") " << value << std::endl;
    return true;
}

bool GribInput::get(const std::string &name, double &value) const {
    const char *key = get_key(name);
    int err = grib_get_double(grib_, key, &value);

    if (err == GRIB_NOT_FOUND) {
        return FieldParametrisation::get(name, value);
    }

    if (err) {
        eckit::Log::info() << "grib_get_double(" << name << ",key=" << key << ") failed " << err << std::endl;
        GRIB_ERROR(err, key);
    }

    eckit::Log::info() << "grib_get_double(" << name << ",key=" << key << ") " << value << std::endl;
    return true;
}

bool GribInput::get(const std::string &name, std::vector<long> &value) const {
    const char *key = get_key(name);

    size_t count = 0;
    int err = grib_get_size(grib_, key, &count);

    if (err == GRIB_NOT_FOUND) {
        return FieldParametrisation::get(name, value);
    }

    if (err) {
        eckit::Log::info() << "grib_get_long_array(" << name << ",key=" << key << ") failed " << err << " count=" << count << std::endl;
        GRIB_ERROR(err, key);
    }

    size_t size = count;

    value.resize(count);

    GRIB_CALL(grib_get_long_array(grib_, key, &value[0], &size));
    ASSERT(count == size);

    ASSERT(value.size());

    eckit::Log::info() << "grib_get_long_array(" << name << ",key=" << key << ") size=" << value.size() << std::endl;


    return true;
}

bool GribInput::get(const std::string &name, std::string &value) const {
    const char *key = get_key(name);

    char buffer[10240];
    size_t size = sizeof(buffer);
    int err = grib_get_string(grib_, key, buffer, &size);

    if (err == GRIB_NOT_FOUND) {
        return FieldParametrisation::get(name, value);
    }

    if (err) {
        eckit::Log::info() << "grib_get_string(" << name << ",key=" << key << ") failed " << err << std::endl;
        GRIB_ERROR(err, key);
    }

    ASSERT(size < sizeof(buffer) - 1);

    if (::strcmp(buffer, "MISSING") == 0) {
        return false;
    }

    value = buffer;

    eckit::Log::info() << "grib_get_string(" << name << ",key=" << key << ") " << value << std::endl;

    return true;
}

bool GribInput::get(const std::string &name, std::vector<double> &value) const {
    const char *key = get_key(name);

    size_t count = 0;
    int err = grib_get_size(grib_, key, &count);

    if (err == GRIB_NOT_FOUND) {
        return FieldParametrisation::get(name, value);
    }

    if (err) {
        eckit::Log::info() << "grib_get_double_array(" << name << ",key=" << key << ") failed " << err << " count=" << count << std::endl;
        GRIB_ERROR(err, key);
    }

    size_t size = count;

    value.resize(count);

    GRIB_CALL(grib_get_double_array(grib_, key, &value[0], &size));
    ASSERT(count == size);

    ASSERT(value.size());

    eckit::Log::info() << "grib_get_double_array(" << name << ",key=" << key << ") size=" << value.size() << std::endl;


    return true;
}

bool GribInput::handle(grib_handle *h) {
    if (grib_) {
        grib_handle_delete(grib_);
    }
    grib_ = h;
    return h != 0;
}


}  // namespace input
}  // namespace mir

