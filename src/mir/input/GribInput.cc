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
#include "mir/repres/Representation.h"

#include <iomanip>

namespace mir {
namespace input {
namespace {

class Condition {
  public:
    virtual bool eval(grib_handle *) const = 0;
};

template<class T>
class ConditionT : public Condition {
    const char *key_;
    T value_;
    virtual bool eval(grib_handle *) const;
  public:
    ConditionT(const char *key, const T &value): key_(key), value_(value) {}
};

template<>
bool ConditionT<long>::eval(grib_handle *h ) const {
    long value;
    int err = grib_get_long(h, key_, &value);

    if (err == GRIB_NOT_FOUND) {
        return false;
    }

    if (err) {
        eckit::Log::info() << "ConditionT<long>::eval(" << ",key=" << key_ << ") failed " << err << std::endl;
        GRIB_ERROR(err, key_);
    }

    return value_ == value;
}

template<>
bool ConditionT<double>::eval(grib_handle *h ) const {
    double value;
    int err = grib_get_double(h, key_, &value);

    if (err == GRIB_NOT_FOUND) {
        return false;
    }

    if (err) {
        eckit::Log::info() << "ConditionT<double>::eval(" << ",key=" << key_ << ") failed " << err << std::endl;
        GRIB_ERROR(err, key_);
    }

    return value_ == value; // Want an epsilon?
}

template<>
bool ConditionT<std::string>::eval(grib_handle *h ) const {
    char buffer[10240];
    size_t size = sizeof(buffer);
    int err = grib_get_string(h, key_, buffer, &size);

    if (err == GRIB_NOT_FOUND) {
        return false;
    }

    if (err) {
        eckit::Log::info() << "ConditionT<std::string>::eval(" << ",key=" << key_ << ") failed " << err << std::endl;
        GRIB_ERROR(err, key_);
    }

    return value_ == buffer;
}

class ConditionOR : public Condition {
    const Condition *left_;
    const Condition *right_;
    virtual bool eval(grib_handle *h) const {
        return left_->eval(h) || right_->eval(h);
    }
  public:
    ConditionOR(const Condition *left, const Condition *right): left_(left), right_(right) {}
};

class ConditionAND : public Condition {
    const Condition *left_;
    const Condition *right_;
    virtual bool eval(grib_handle *h) const {
        return left_->eval(h) && right_->eval(h);
    }
  public:
    ConditionAND(const Condition *left, const Condition *right): left_(left), right_(right) {}
};

class ConditionNOT : public Condition {
    const Condition *c_;
    virtual bool eval(grib_handle *h) const {
        return !c_->eval(h);
    }
  public:
    ConditionNOT(const Condition *c) : c_(c) {}
};

template<class T>
static Condition *is(const char *key, const T &value) {
    return new ConditionT<T>(key, value);
}

static Condition *is(const char *key, const char *value) {
    return new ConditionT<std::string>(key, value);
}

static Condition *_and(const Condition *left, const Condition *right) {
    return new ConditionAND(left, right);
}

static Condition *_or(const Condition *left, const Condition *right) {
    return new ConditionOR(left, right);
}

static Condition *_not(const Condition *c) {
    return new ConditionNOT(c);
}

static struct {
    const char *name;
    const char *key;
    const Condition *condition;
} mappings[] = {
    {"west_east_increment", "iDirectionIncrementInDegrees"},
    {"south_north_increment", "jDirectionIncrementInDegrees"},

    {"west", "longitudeOfFirstGridPointInDegrees"},
    {"east", "longitudeOfLastGridPointInDegrees"},

    {"north", "latitudeOfFirstGridPointInDegrees", is("scanningMode", 0L)},
    {"south", "latitudeOfLastGridPointInDegrees", is("scanningMode", 0L)},

    {"north", "latitudeOfLastGridPointInDegrees", is("jScansPositively", 1L)},
    {"south", "latitudeOfFirstGridPointInDegrees", is("jScansPositively", 1L)},

    {"truncation", "pentagonalResolutionParameterJ",},  // Assumes triangular truncation

    {"south_pole_latitude", "latitudeOfSouthernPoleInDegrees"},
    {"south_pole_longitude", "longitudeOfSouthernPoleInDegrees"},
    {"south_pole_rotation_angle", "angleOfRotationInDegrees"},

    // This will be just called for has()
    {"gridded", "Nx", is("dataRepresentationType", 5L),},  // Polar stereo
    {"gridded", "numberOfPointsAlongAMeridian"},  // Is that always true?
    {"spherical", "pentagonalResolutionParameterJ"},

    /// FIXME: Find something that does no clash
    {"reduced", "numberOfParallelsBetweenAPoleAndTheEquator",  is("isOctahedral", 0L)},
    {"regular", "N", is("gridType", "regular_gg")},
    {"octahedral", "numberOfParallelsBetweenAPoleAndTheEquator", is("isOctahedral", 1L)},


    {0, 0},
};


static const char *get_key(const std::string &name, grib_handle *h) {
    const char *key = name.c_str();
    size_t i = 0;
    while (mappings[i].name) {
        if (name == mappings[i].name) {
            if (mappings[i].condition == 0 || mappings[i].condition->eval(h)) {
                return mappings[i].key;
            }
        }
        i++;
    }
    return key;
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



    data::MIRField *field = new data::MIRField(*this, bitmap != 0, missing);

    long scanningMode = 0;
    if (grib_get_long(grib_, "scanningMode", &scanningMode) == GRIB_SUCCESS && scanningMode != 0) {
        field->representation()->reorder(scanningMode, values);
    }

    field->values(values, 0);
    return field;

}

grib_handle *GribInput::gribHandle() const {
    return grib_;
}

bool GribInput::has(const std::string &name) const {
    const char *key = get_key(name, grib_);

    bool    ok = grib_is_defined(grib_, key);

    eckit::Log::info() << "GribInput::has(" << name << ",key=" << key << ") " << (ok ? "yes" : "no") << std::endl;
    return ok;
}

bool GribInput::get(const std::string &name, bool &value) const {
    long temp;
    const char *key = get_key(name, grib_);
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
    const char *key = get_key(name, grib_);
    int err = grib_get_long(grib_, key, &value);

    if (err == GRIB_NOT_FOUND) {
        return FieldParametrisation::get(name, value);
    }

    if (err) {
        eckit::Log::info() << "grib_get_long(" << name << ",key=" << key << ") failed " << err << std::endl;
        GRIB_ERROR(err, key);
    }

    // eckit::Log::info() << "grib_get_long(" << name << ",key=" << key << ") " << value << std::endl;
    return true;
}

bool GribInput::get(const std::string &name, double &value) const {
    const char *key = get_key(name, grib_);
    int err = grib_get_double(grib_, key, &value);

    if (err == GRIB_NOT_FOUND) {
        return FieldParametrisation::get(name, value);
    }

    if (err) {
        eckit::Log::info() << "grib_get_double(" << name << ",key=" << key << ") failed " << err << std::endl;
        GRIB_ERROR(err, key);
    }

    // eckit::Log::info() << "grib_get_double(" << name << ",key=" << key << ") " << value << std::endl;
    return true;
}

bool GribInput::get(const std::string &name, std::vector<long> &value) const {
    const char *key = get_key(name, grib_);

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
    const char *key = get_key(name, grib_);

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
    const char *key = get_key(name, grib_);

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

