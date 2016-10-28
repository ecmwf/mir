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


#include <iomanip>
#include <iostream>

#include "eckit/config/Resource.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/io/BufferedHandle.h"
#include "eckit/io/MemoryHandle.h"
#include "eckit/io/StdFile.h"
#include "eckit/serialisation/HandleStream.h"
#include "eckit/thread/AutoLock.h"

#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/input/GribFileInput.h"
#include "mir/input/GribInput.h"
#include "mir/param/MIRConfiguration.h"
#include "mir/repres/Representation.h"
#include "mir/util/Grib.h"


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
    ASSERT(h);
    int err = grib_get_long(h, key_, &value);

    if (err == GRIB_NOT_FOUND) {
        return false;
    }

    if (err) {
        // eckit::Log::debug<LibMir>() << "ConditionT<long>::eval(" << ",key=" << key_ << ") failed " << err << std::endl;
        GRIB_ERROR(err, key_);
    }

    return value_ == value;
}

template<>
bool ConditionT<double>::eval(grib_handle *h ) const {
    double value;
    ASSERT(h);
    int err = grib_get_double(h, key_, &value);

    if (err == GRIB_NOT_FOUND) {
        return false;
    }

    if (err) {
        // eckit::Log::debug<LibMir>() << "ConditionT<double>::eval(" << ",key=" << key_ << ") failed " << err << std::endl;
        GRIB_ERROR(err, key_);
    }

    return value_ == value; // Want an epsilon?
}

template<>
bool ConditionT<std::string>::eval(grib_handle *h ) const {
    char buffer[10240];
    size_t size = sizeof(buffer);
    ASSERT(h);
    int err = grib_get_string(h, key_, buffer, &size);

    if (err == GRIB_NOT_FOUND) {
        return false;
    }

    if (err) {
        eckit::Log::debug<LibMir>() << "ConditionT<std::string>::eval(" << ",key=" << key_ << ") failed " << err << std::endl;
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

/*
static Condition *_and(const Condition *left, const Condition *right) {
    return new ConditionAND(left, right);
}

static Condition *_or(const Condition *left, const Condition *right) {
    return new ConditionOR(left, right);
}

static Condition *_not(const Condition *c) {
    return new ConditionNOT(c);
}
*/

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
    {"gridded", "Nx", is("gridType", "polar_stereographic"),},  // Polar stereo
    {"gridded", "Ni", is("gridType", "triangular_grid"),},  // Polar stereo
    {"gridded", "numberOfGridInReference", is("gridType", "unstructured_grid"),},  // numberOfGridInReference is just dummy

    {"gridded", "numberOfPointsAlongAMeridian"},  // Is that always true?

    {"spectral", "pentagonalResolutionParameterJ"},

    /// FIXME: Find something that does no clash
    {"reduced", "numberOfParallelsBetweenAPoleAndTheEquator",  is("isOctahedral", 0L)},
    {"regular", "N", is("gridType", "regular_gg")},
    {"octahedral", "numberOfParallelsBetweenAPoleAndTheEquator", is("isOctahedral", 1L)},

    /// TODO: is that a good idea?
    {"param", "paramId"},

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


const param::MIRParametrisation &GribInput::parametrisation(size_t which) const {
    ASSERT(which == 0);
    return *this;
}


data::MIRField GribInput::field() const {

    // Protect the grib_handle, as eccodes may update its internals
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    ASSERT(grib_);
    // TODO: this is only here for debugging purposes
    // GRIB_CALL(grib_set_double(grib_, "missingValue", 1.e15));

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

    data::MIRField field(*this, bitmap != 0, missing);

    long scanningMode = 0;
    if (grib_get_long(grib_, "scanningMode", &scanningMode) == GRIB_SUCCESS && scanningMode != 0) {
        field.representation()->reorder(scanningMode, values);
    }

    field.update(values, 0);
    field.validate();

    return field;
}


grib_handle *GribInput::gribHandle(size_t which) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    ASSERT(which == 0);
    return grib_;
}


bool GribInput::has(const std::string &name) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    ASSERT(grib_);
    const char *key = get_key(name, grib_);

    bool    ok = grib_is_defined(grib_, key);

    // eckit::Log::debug<LibMir>() << "GribInput::has(" << name << ",key=" << key << ") " << (ok ? "yes" : "no") << std::endl;
    return ok;
}


bool GribInput::get(const std::string &name, bool &value) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    ASSERT(grib_);
    long temp = GRIB_MISSING_LONG;
    const char *key = get_key(name, grib_);
    int err = grib_get_long(grib_, key, &temp);

    if (err == GRIB_NOT_FOUND || temp == GRIB_MISSING_LONG) {
        return FieldParametrisation::get(name, value);
    }

    if (err) {
        // eckit::Log::debug<LibMir>() << "grib_get_bool(" << name << ",key=" << key << ") failed " << err << std::endl;
        GRIB_ERROR(err, key);
    }

    value = temp != 0;

    // eckit::Log::debug<LibMir>() << "grib_get_bool(" << name << ",key=" << key << ") " << value << std::endl;
    return true;
}


bool GribInput::get(const std::string &name, long &value) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    ASSERT(grib_);
    const char *key = get_key(name, grib_);
    int err = grib_get_long(grib_, key, &value);

    // FIXME: make sure that 'value' is not set if GRIB_MISSING_LONG
    if (err == GRIB_NOT_FOUND || value == GRIB_MISSING_LONG) {
        return FieldParametrisation::get(name, value);
    }

    if (err) {
        eckit::Log::debug<LibMir>() << "grib_get_long(" << name << ",key=" << key << ") failed " << err << std::endl;
        GRIB_ERROR(err, key);
    }

    // eckit::Log::debug<LibMir>() << "grib_get_long(" << name << ",key=" << key << ") " << value << std::endl;
    return true;
}


bool GribInput::get(const std::string &name, double &value) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    ASSERT(grib_);
    const char *key = get_key(name, grib_);
    int err = grib_get_double(grib_, key, &value);

    // FIXME: make sure that 'value' is not set if GRIB_MISSING_DOUBLE
    if (err == GRIB_NOT_FOUND || value == GRIB_MISSING_DOUBLE) {
        return FieldParametrisation::get(name, value);
    }

    if (err) {
        // eckit::Log::debug<LibMir>() << "grib_get_double(" << name << ",key=" << key << ") failed " << err << std::endl;
        GRIB_ERROR(err, key);
    }

    // eckit::Log::debug<LibMir>() << "grib_get_double(" << name << ",key=" << key << ") " << value << std::endl;
    return true;
}


bool GribInput::get(const std::string &name, std::vector<long> &value) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    ASSERT(grib_);
    const char *key = get_key(name, grib_);

    size_t count = 0;
    int err = grib_get_size(grib_, key, &count);

    if (err == GRIB_NOT_FOUND) {
        return FieldParametrisation::get(name, value);
    }

    if (err) {
        eckit::Log::debug<LibMir>() << "grib_get_long_array(" << name << ",key=" << key << ") failed " << err << " count=" << count << std::endl;
        GRIB_ERROR(err, key);
    }

    size_t size = count;

    value.resize(count);

    GRIB_CALL(grib_get_long_array(grib_, key, &value[0], &size));
    ASSERT(count == size);

    ASSERT(value.size());

    // eckit::Log::debug<LibMir>() << "grib_get_long_array(" << name << ",key=" << key << ") size=" << value.size() << std::endl;


    return true;
}


bool GribInput::get(const std::string &name, std::string &value) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    ASSERT(grib_);
    const char *key = get_key(name, grib_);

    char buffer[10240];
    size_t size = sizeof(buffer);
    int err = grib_get_string(grib_, key, buffer, &size);

    if (err == GRIB_NOT_FOUND) {
        return FieldParametrisation::get(name, value);
    }

    if (err) {
        // eckit::Log::debug<LibMir>() << "grib_get_string(" << name << ",key=" << key << ") failed " << err << std::endl;
        GRIB_ERROR(err, key);
    }

    // std::cout << err << "  " << size << " " << name << std::endl;

    ASSERT(size < sizeof(buffer) - 1);

    if (::strcmp(buffer, "MISSING") == 0) {
        return false;
    }

    value = buffer;

    // eckit::Log::debug<LibMir>() << "grib_get_string(" << name << ",key=" << key << ") " << value << std::endl;

    return true;
}


bool GribInput::get(const std::string &name, std::vector<double> &value) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    ASSERT(grib_);
    const char *key = get_key(name, grib_);

    size_t count = 0;
    int err = grib_get_size(grib_, key, &count);

    if (err == GRIB_NOT_FOUND) {
        return FieldParametrisation::get(name, value);
    }

    if (err) {
        // eckit::Log::debug<LibMir>() << "grib_get_double_array(" << name << ",key=" << key << ") failed " << err << " count=" << count << std::endl;
        GRIB_ERROR(err, key);
    }

    size_t size = count;

    value.resize(count);

    GRIB_CALL(grib_get_double_array(grib_, key, &value[0], &size));
    ASSERT(count == size);

    ASSERT(value.size());

    // eckit::Log::debug<LibMir>() << "grib_get_double_array(" << name << ",key=" << key << ") size=" << value.size() << std::endl;


    return true;
}


bool GribInput::handle(grib_handle *h) {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    if (grib_) {
        grib_handle_delete(grib_);
    }
    grib_ = h;
    return h != 0;
}


void GribInput::auxilaryValues(const std::string &path, std::vector<double> &values) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    eckit::StdFile f(path);
    int e;
    grib_handle *h = 0;

    // We cannot use GribFileInput to read these files, because lat/lon files are also
    // has grid_type = triangular_grid, and we will create a loop

    try {
        h = grib_handle_new_from_file(0, f, &e);
        grib_call(e, path.c_str());
        size_t count;
        GRIB_CALL(grib_get_size(h, "values", &count));

        size_t size = count;
        values.resize(count);
        GRIB_CALL(grib_get_double_array(h, "values", &values[0], &size));
        ASSERT(count == size);

        long bitmap;
        GRIB_CALL(grib_get_long(h, "bitmapPresent", &bitmap));
        ASSERT(!bitmap);

        grib_handle_delete(h);
    } catch (...) {
        if (h) grib_handle_delete(h);
        throw;
    }
}


void GribInput::setAuxilaryFiles(const std::string &pathToLatitudes, const std::string &pathToLongitudes) {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    // eckit::Log::debug<LibMir>() << "Loading auxilary files " << pathToLatitudes << " and " << pathToLongitudes << std::endl;
    auxilaryValues(pathToLatitudes, latitudes_);
    auxilaryValues(pathToLongitudes, longitudes_);
}


// TODO: some caching, also next() should maybe advance the auxilary files
void GribInput::latitudes(std::vector<double> &values) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    values.clear();
    values.reserve(latitudes_.size());
    std::copy(latitudes_.begin(), latitudes_.end(), std::back_inserter(values));
}


void GribInput::longitudes(std::vector<double> &values) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    values.clear();
    values.reserve(longitudes_.size());
    std::copy(longitudes_.begin(), longitudes_.end(), std::back_inserter(values));
}


void GribInput::marsRequest(std::ostream &out) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    ASSERT(grib_);

    static std::string gribToRequestNamespace = eckit::Resource<std::string>("gribToRequestNamespace", "mars");

    grib_keys_iterator *keys =  grib_keys_iterator_new(grib_, GRIB_KEYS_ITERATOR_ALL_KEYS, gribToRequestNamespace.c_str());
    ASSERT(keys);

    const char *sep = "";
    try {
        while (grib_keys_iterator_next(keys)) {

            char value[1024];
            size_t size = sizeof(value);
            out << sep << grib_keys_iterator_get_name(keys);
            GRIB_CALL(grib_keys_iterator_get_string(keys, value, &size));
            out << "=" << value;
            sep = ",";
        }
        grib_keys_iterator_delete(keys);
        keys = 0;

        size_t size = 0;
        int err = grib_get_size(grib_, "freeFormData", &size);

        if (err == 0) {
            eckit::Buffer buffer(size);
            char *b = buffer;

            GRIB_CALL(grib_get_bytes(grib_ , "freeFormData", (unsigned char *)b, &size));
            ASSERT(size == buffer.size());

            eckit::MemoryHandle h(buffer);
            eckit::HandleStream in(h);
            int n;
            in >> n; // Number of requests
            ASSERT(n == 1);
            std::string verb;
            in >> verb;

            in >> n;
            for (int i = 0; i < n ; i++) {
                std::string param;
                in >> param;
                out << sep << param;
                const char *slash = "=";
                int m;
                in >> m;
                for (int j = 0; j < m; j++) {
                    std::string value;
                    in >> value;
                    out << slash << value;
                    slash = "/";
                }

            }

            NOTIMP;
        }


        if (err != GRIB_NOT_FOUND) {
            grib_call(err, "freeFormData");
        }

    } catch (...) {
        if (keys) {
            grib_keys_iterator_delete(keys);
        }
        throw;
    }
}


}  // namespace input
}  // namespace mir

