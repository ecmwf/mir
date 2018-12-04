/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "mir/input/GribInput.h"

#include <algorithm>
#include <functional>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <sstream>

#include "eckit/config/Resource.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/io/AutoCloser.h"
#include "eckit/io/BufferedHandle.h"
#include "eckit/io/MemoryHandle.h"
#include "eckit/io/StdFile.h"
#include "eckit/memory/NonCopyable.h"
#include "eckit/serialisation/HandleStream.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/types/Fraction.h"

#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/repres/Representation.h"
#include "mir/util/Grib.h"
#include "mir/util/Wind.h"


namespace mir {
namespace input {


namespace {


class Condition {
public:
    virtual ~Condition() = default;
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
    {"east", "longitudeOfLastGridPointInDegrees_fix_for_global_reduced_grids"},

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
    {"gridded", "numberOfPointsAlongXAxis", is("gridType", "lambert_azimuthal_equal_area"),},
    {"gridded", "numberOfGridInReference", is("gridType", "unstructured_grid"),},  // numberOfGridInReference is just dummy

    {"gridded", "numberOfPointsAlongAMeridian"},  // Is that always true?

    {"gridname", "gridName"},
    {"angularPrecision", "angularPrecisionInDegrees"},

    {"spectral", "pentagonalResolutionParameterJ"},

    /// FIXME: Find something that does no clash
    {"reduced", "numberOfParallelsBetweenAPoleAndTheEquator",  is("isOctahedral", 0L)},
    {"regular", "N", is("gridType", "regular_gg")},
    {"octahedral", "numberOfParallelsBetweenAPoleAndTheEquator", is("isOctahedral", 1L)},

    /// TODO: is that a good idea?
    {"param", "paramId"},

    {nullptr, nullptr},
};


struct Processing : eckit::NonCopyable {
    virtual ~Processing() = default;
    virtual void eval(grib_handle*, double&) const { NOTIMP; }
    virtual void eval(grib_handle*, long&) const { NOTIMP; }
};

template<typename T>
struct ProcessingT : Processing {
    using fun_t = std::function<T(grib_handle*)>;
    fun_t fun_;
    ProcessingT(fun_t&& fun) : fun_(fun) {}
    void eval(grib_handle* h, T& v) const override {
        v = fun_(h);
    }
};

static ProcessingT<long>* is_wind_component_uv() {
    return new ProcessingT<long>([=](grib_handle* h) {
        long paramId = 0;
        GRIB_CALL(grib_get_long(h, "paramId", &paramId));
        static const util::Wind::Defaults def;
        long ind = paramId % 1000;
        return ind == def.u ? 1 : ind == def.v ? 2 : 0;
    });
}

static ProcessingT<long>* is_wind_component_vod() {
    return new ProcessingT<long>([=](grib_handle* h) {
        long paramId = 0;
        GRIB_CALL(grib_get_long(h, "paramId", &paramId));
        static const util::Wind::Defaults def;
        long ind = paramId % 1000;
        return ind == def.vo ? 1 : ind == def.d ? 2 : 0;
    });
}

static ProcessingT<double>* inverse(const char *key) {
    return new ProcessingT<double>([=](grib_handle* h) {
        double value = 0;
        GRIB_CALL(grib_get_double(h, key, &value));
        ASSERT(!eckit::types::is_approximately_equal<double>(value, 0));
        return 1 / value;
    });
}

static ProcessingT<double>* longitudeOfLastGridPointInDegrees_fix_for_global_reduced_grids() {
    return new ProcessingT<double>([](grib_handle* h) {

        double Lon2 = 0;
        GRIB_CALL(grib_get_double(h, "longitudeOfLastGridPointInDegrees", &Lon2));

        if (grib_is_defined(h, "pl")) {

            double Lon1 = 0;
            GRIB_CALL(grib_get_double(h, "longitudeOfFirstGridPointInDegrees", &Lon1));

            if (eckit::types::is_approximately_equal<double>(Lon1, 0)) {

                // get pl array maximum and sum
                // if sum equals values size the grid must be global
                size_t plSize = 0;
                GRIB_CALL(grib_get_size(h, "pl", &plSize));
                ASSERT(plSize);

                std::vector<long> pl(plSize, 0);
                size_t plSizeAsRead = plSize;
                GRIB_CALL(grib_get_long_array(h, "pl", pl.data(), &plSizeAsRead));
                ASSERT(plSize == plSizeAsRead);

                long plMax = 0;
                long plSum = 0;
                for (auto& p : pl) {
                    plSum += p;
                    if (plMax < p) {
                        plMax = p;
                    }
                }
                ASSERT(plMax > 0);

                size_t valuesSize;
                GRIB_CALL(grib_get_size(h, "values", &valuesSize));

                if (size_t(plSum) == valuesSize) {

                    long angularPrecision;
                    GRIB_CALL(grib_get_long(h, "angularPrecision", &angularPrecision));
                    ASSERT(angularPrecision > 0);

                    eckit::Fraction eps(1L, angularPrecision);
                    eckit::Fraction Lon2_expected(360L * (plMax - 1L), plMax);

                    if (!eckit::types::is_approximately_equal<double>(Lon2, Lon2_expected, eps)) {

                        std::ostringstream msgs;
                        msgs.precision(32);
                        msgs << "GribInput: longitudeOfLastGridPointInDegrees is wrongly encoded (reduced_gg):"
                             << "\n" "encoded:  " << Lon2
                             << "\n" "expected: " << Lon2_expected
                             << std::endl;
                        const std::string msg(msgs.str());

                        static bool abortIfWronglyEncodedGRIB = eckit::Resource<bool>("$MIR_ABORT_IF_WRONGLY_ENCODED_GRIB", false);
                        if (abortIfWronglyEncodedGRIB) {
                            eckit::Log::error() << msg << std::endl;
                            throw eckit::UserError(msg);
                        }

                        eckit::Log::warning() << msg << std::endl;
                        Lon2 = Lon2_expected;
                    }
                }

            }
        }

        return Lon2;
    });
};

static ProcessingT<double>* divide(const char *key, double denominator) {
    ASSERT(eckit::types::is_strictly_greater<double>(denominator, 0));
    return new ProcessingT<double>([=](grib_handle* h) {
        double value = 0;
        GRIB_CALL(grib_get_double(h, key, &value));
        return value / denominator;
    });
}

static struct {
    const char *name;
    const Processing *processing;
} processings[] = {
    {"angularPrecisionInDegrees", inverse("angularPrecision")},
    {"xDirectionGridLengthInMetres", divide("xDirectionGridLengthInMillimetres", 1000.)},
    {"yDirectionGridLengthInMetres", divide("yDirectionGridLengthInMillimetres", 1000.)},
    {"standardParallelInDegrees", divide("standardParallelInMicrodegrees", 1000000.)},
    {"centralLongitudeInDegrees", divide("centralLongitudeInMicrodegrees", 1000000.)},
    {"longitudeOfLastGridPointInDegrees_fix_for_global_reduced_grids", longitudeOfLastGridPointInDegrees_fix_for_global_reduced_grids()},
    {"is_wind_component_uv", is_wind_component_uv()},
    {"is_wind_component_vod", is_wind_component_vod()},
    {nullptr, nullptr},
};


static const char *get_key(const std::string &name, grib_handle *h) {
    const char *key = name.c_str();
    size_t i = 0;
    while (mappings[i].name) {
        if (name == mappings[i].name) {
            if (mappings[i].condition == nullptr || mappings[i].condition->eval(h)) {
                return mappings[i].key;
            }
        }
        i++;
    }
    return key;
}


void get_unique_missing_value(const MIRValuesVector& values, double& missing) {
    ASSERT(values.size());

    if (!std::count(values.begin(), values.end(), missing)) {
        return;
    }

    missing = *std::max_element(values.begin(), values.end()) + 1.;
    if (missing == missing) {
        return;
    }

    missing = *std::min_element(values.begin(), values.end()) - 1.;
    if (missing == missing) {
        return;
    }

    throw eckit::SeriousBug("GribInput: get_unique_missing_value: failed to get a unique missing value.");
}


size_t fix_pl_array_zeros(std::vector<long>& pl) {
    size_t new_entries = 0;

    // if a zero is found, copy the *following* non-zero value into the range "current entry -> non-zero entry"
    for (auto p = pl.begin(); p != pl.end(); ++p) {
        if (*p == 0) {

            auto nz = std::find_if(p, pl.end(), [](long x) { return x != 0; });
            if (nz != pl.end()) {
                new_entries += size_t(*nz) * size_t(std::distance(p, nz));
                std::fill(p, nz, *nz);
            }

        }
    }

    // if a zero is found, copy the *previous* non-zero value into the range "non-zero entry -> current entry"
    for (auto p = pl.rbegin(); p != pl.rend(); ++p) {
        if (*p == 0) {

            auto nz = std::find_if(p, pl.rend(), [](long x) { return x != 0; });
            if (nz != pl.rend()) {
                new_entries += size_t(*nz) * size_t(std::distance(p, nz));
                std::fill(p, nz, *nz);
            }

        }
    }

    ASSERT(0 == std::count(pl.begin(), pl.end(), 0));
    ASSERT(new_entries);
    return new_entries;
}



}  // (anonymous namespace)


GribInput::GribInput():
    cache_(*this),
    grib_(nullptr) {
}


GribInput::~GribInput() {
    handle(nullptr); // Will delete handle
}


const param::MIRParametrisation &GribInput::parametrisation(size_t which) const {
    ASSERT(which == 0);
    return cache_;
}


data::MIRField GribInput::field() const {

    // Protect the grib_handle, as eccodes may update its internals
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    ASSERT(grib_);

    long localDefinitionNumber = 0;
    if (grib_get_long(grib_, "localDefinitionNumber", &localDefinitionNumber) == GRIB_SUCCESS) {
        if (localDefinitionNumber == 4) {
            throw eckit::UserError("GribInput: GRIB localDefinitionNumber=4 ('ocean') not supported");
        }
    }

    size_t count;
    GRIB_CALL(grib_get_size(grib_, "values", &count));

    size_t size = count;
    MIRValuesVector values(count);
    GRIB_CALL(grib_get_double_array(grib_, "values", &values[0], &size));
    ASSERT(count == size);

    long missingValuesPresent;
    GRIB_CALL(grib_get_long(grib_, "missingValuesPresent", &missingValuesPresent));

    double missing;
    GRIB_CALL(grib_get_double(grib_, "missingValue", &missing));

    // If grib has a 0-containing pl array, add missing values in their place
    if (has("pl")) {
        size_t count_pl = 0;
        GRIB_CALL(grib_get_size(grib_, "pl", &count_pl));
        ASSERT(count_pl);

        std::vector<long> pl(count_pl, 0);
        size = count_pl;
        GRIB_CALL(grib_get_long_array(grib_, "pl", pl.data(), &size));
        ASSERT(count_pl == size);

        // NOTE: this fix ties with the method get(const std::string &name, std::vector<long> &value)
        if (std::find(pl.rbegin(), pl.rend(), 0) != pl.rend()) {

            // if there are no missing values yet, set them
            if (!missingValuesPresent) {
                eckit::Log::debug<LibMir>() << "GribInput::field(): introducing missing values (setting bitmap)." << std::endl;
                missingValuesPresent = 1;
                get_unique_missing_value(values, missing);
            }

            // pl array: insert entries in place of zeros
            std::vector<long> pl_fixed = pl;
            size_t new_values = fix_pl_array_zeros(pl_fixed);
            ASSERT(new_values > 0);


            // values array: copy values row by row, and when a fixed (0) entry is found, insert missing values
            eckit::Log::debug<LibMir>() << "GribInput::field(): correcting values array with " << new_values << " new missing values." << std::endl;

            MIRValuesVector values_extended;
            values_extended.reserve(count + new_values);

            ASSERT(pl.size() == pl_fixed.size());
            size_t i = 0;
            for (auto p1 = pl.begin(), p2 = pl_fixed.begin(); p1 != pl.end(); ++p1, ++p2) {
                if (*p1 == 0) {
                    ASSERT(*p2 > 0);
                    size_t Ni = size_t(*p2);

                    values_extended.insert(values_extended.end(), Ni, missing);

                } else {
                    size_t Ni = size_t(*p1);
                    ASSERT(i + Ni <= count);

                    values_extended.insert(values_extended.end(), &values[i], &values[i + Ni]);
                    i += Ni;

                }
            }

            // set the new (extended) values vector, confirm it is compatible with a returned pl array
            ASSERT(values.size() + new_values == values_extended.size());
            values.swap(values_extended);

            ASSERT(get("pl", pl));
            size_t pl_sum = size_t(std::accumulate(pl.begin(), pl.end(), 0));
            ASSERT(pl_sum == values.size());
        }
    }

    data::MIRField field(*this, missingValuesPresent != 0, missing);

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


bool GribInput::has(const std::string& name) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    ASSERT(grib_);
    const char *key = get_key(name, grib_);

    bool    ok = grib_is_defined(grib_, key);

    // eckit::Log::debug<LibMir>() << "GribInput::has(" << name << ",key=" << key << ") " << (ok ? "yes" : "no") << std::endl;
    return ok;
}


bool GribInput::get(const std::string& name, bool& value) const {
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


bool GribInput::get(const std::string& name, int& value) const {
    long v;
    if (get(name, v)) {
        ASSERT(long(int(v)) == v);
        value = int(v);
        return true;
    }
    return false;
}


bool GribInput::get(const std::string& name, long& value) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    ASSERT(grib_);
    const char *key = get_key(name, grib_);
    int err = grib_get_long(grib_, key, &value);

    // FIXME: make sure that 'value' is not set if GRIB_MISSING_LONG
    if (err == GRIB_NOT_FOUND || value == GRIB_MISSING_LONG) {

        for (size_t i = 0; processings[i].name; ++i) {
            if (std::string(key) == processings[i].name) {
                ASSERT(processings[i].processing);
                processings[i].processing->eval(grib_, value);
                return true;
            }
        }

        return FieldParametrisation::get(name, value);
    }

    if (err) {
        eckit::Log::debug<LibMir>() << "grib_get_long(" << name << ",key=" << key << ") failed " << err << std::endl;
        GRIB_ERROR(err, key);
    }

    // eckit::Log::debug<LibMir>() << "grib_get_long(" << name << ",key=" << key << ") " << value << std::endl;
    return true;
}


bool GribInput::get(const std::string& name, float& value) const {
    double v;
    if (get(name, v)) {
        value = float(v);
        return true;
    }
    return false;

}


bool GribInput::get(const std::string& name, double& value) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    ASSERT(grib_);
    const std::string key = get_key(name, grib_);
    int err = grib_get_double(grib_, key.c_str(), &value);

    // FIXME: make sure that 'value' is not set if GRIB_MISSING_DOUBLE
    if (err == GRIB_NOT_FOUND || value == GRIB_MISSING_DOUBLE) {

        for (size_t i = 0; processings[i].name; ++i) {
            if (std::string(key) == processings[i].name) {
                ASSERT(processings[i].processing);
                processings[i].processing->eval(grib_, value);
                return true;
            }
        }

        return FieldParametrisation::get(name, value);
    }

    if (err) {
        // eckit::Log::debug<LibMir>() << "grib_get_double(" << name << ",key=" << key << ") failed " << err << std::endl;
        GRIB_ERROR(err, key.c_str());
    }

    // eckit::Log::debug<LibMir>() << "grib_get_double(" << name << ",key=" << key << ") " << value << std::endl;
    return true;
}


bool GribInput::get(const std::string& name, std::vector<int>& value) const {
    std::vector<long> v;
    if (get(name, v)) {
        value.clear();
        value.reserve(v.size());
        for (const long& l : v) {
            ASSERT(long(int(l)) == l);
            value.push_back(int(l));
        }
        return true;
    }
    return false;
}


bool GribInput::get(const std::string& name, std::vector<long>& value) const {
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
    if (name == "pl") {

        // try locating a zero in the pl array, in which case adjust it before continuing
        // NOTE: reverse search is more suitable for latest global reduced_ll grids
        if (std::find(value.rbegin(), value.rend(), 0) != value.rend()) {
            fix_pl_array_zeros(value);
        }

    }

    return true;
}


bool GribInput::get(const std::string& name, std::vector<float>& value) const {
    std::vector<double> v;
    if (get(name, v)) {
        value.clear();
        value.reserve(v.size());
        for (const double& l : v) {
            ASSERT(l >= 0);
            value.push_back(float(l));
        }
        return true;
    }
    return false;
}


bool GribInput::get(const std::string& name, std::string& value) const {
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

    // eckit::Log::info() << err << "  " << size << " " << name << std::endl;

    ASSERT(size < sizeof(buffer) - 1);

    if (::strcmp(buffer, "MISSING") == 0) {
        return false;
    }

    value = buffer;

    // eckit::Log::debug<LibMir>() << "grib_get_string(" << name << ",key=" << key << ") " << value << std::endl;

    return true;
}


bool GribInput::get(const std::string& name, std::vector<double>& value) const {
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

bool GribInput::get(const std::string&, std::vector<std::string>&) const {
    NOTIMP;
}


bool GribInput::handle(grib_handle *h) {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    FieldParametrisation::reset();

    cache_.reset();

    if (grib_) {
        grib_handle_delete(grib_);
    }
    grib_ = h;
    return h != nullptr;
}


void GribInput::auxilaryValues(const std::string& path, std::vector<double>& values) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    eckit::AutoStdFile f(path);

    int e;
    grib_handle *h = nullptr;

    // We cannot use GribFileInput to read these files, because lat/lon files are also
    // has grid_type = triangular_grid, and we will create a loop

    try {
        h = grib_handle_new_from_file(nullptr, f, &e);
        grib_call(e, path.c_str());
        size_t count;
        GRIB_CALL(grib_get_size(h, "values", &count));

        size_t size = count;
        values.resize(count);
        GRIB_CALL(grib_get_double_array(h, "values", &values[0], &size));
        ASSERT(count == size);

        long missingValuesPresent;
        GRIB_CALL(grib_get_long(h, "missingValuesPresent", &missingValuesPresent));
        ASSERT(!missingValuesPresent);

        grib_handle_delete(h);
    } catch (...) {
        if (h) {
            grib_handle_delete(h);
        }
        throw;
    }
}


void GribInput::setAuxilaryFiles(const std::string& pathToLatitudes, const std::string& pathToLongitudes) {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    // eckit::Log::debug<LibMir>() << "Loading auxilary files " << pathToLatitudes << " and " << pathToLongitudes << std::endl;
    auxilaryValues(pathToLatitudes, latitudes_);
    auxilaryValues(pathToLongitudes, longitudes_);
}


// TODO: some caching, also next() should maybe advance the auxilary files
void GribInput::latitudes(std::vector<double>& values) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    values.clear();
    values.reserve(latitudes_.size());
    std::copy(latitudes_.begin(), latitudes_.end(), std::back_inserter(values));
}


void GribInput::longitudes(std::vector<double>& values) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    values.clear();
    values.reserve(longitudes_.size());
    std::copy(longitudes_.begin(), longitudes_.end(), std::back_inserter(values));
}


void GribInput::marsRequest(std::ostream& out) const {
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
        keys = nullptr;

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

