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


#include "mir/input/GribInput.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <numeric>
#include <sstream>

#include "eckit/config/Resource.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/io/Buffer.h"
#include "eckit/io/MemoryHandle.h"
#include "eckit/io/StdFile.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/serialisation/HandleStream.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/types/Fraction.h"

#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/repres/Representation.h"
#include "mir/util/Grib.h"
#include "mir/util/LongitudeDouble.h"
#include "mir/util/Wind.h"


namespace mir {
namespace input {


namespace {


class Condition {
public:
    virtual ~Condition()                  = default;
    virtual bool eval(grib_handle*) const = 0;
};

template <class T>
class ConditionT : public Condition {
    const char* key_;
    T value_;
    virtual bool eval(grib_handle*) const;

public:
    ConditionT(const char* key, const T& value) : key_(key), value_(value) {}
};

template <>
bool ConditionT<long>::eval(grib_handle* h) const {
    long value;
    ASSERT(h);
    int err = grib_get_long(h, key_, &value);

    if (err == GRIB_NOT_FOUND) {
        return false;
    }

    if (err) {
        // eckit::Log::debug<LibMir>() << "ConditionT<long>::eval(" << ",key=" << key_ << ") failed " << err <<
        // std::endl;
        GRIB_ERROR(err, key_);
    }

    return value_ == value;
}

template <>
bool ConditionT<double>::eval(grib_handle* h) const {
    double value;
    ASSERT(h);
    int err = grib_get_double(h, key_, &value);

    if (err == GRIB_NOT_FOUND) {
        return false;
    }

    if (err) {
        // eckit::Log::debug<LibMir>() << "ConditionT<double>::eval(" << ",key=" << key_ << ") failed " << err <<
        // std::endl;
        GRIB_ERROR(err, key_);
    }

    return value_ == value;  // Want an epsilon?
}

template <>
bool ConditionT<std::string>::eval(grib_handle* h) const {
    char buffer[10240];
    size_t size = sizeof(buffer);
    ASSERT(h);
    int err = grib_get_string(h, key_, buffer, &size);

    if (err == GRIB_NOT_FOUND) {
        return false;
    }

    if (err) {
        eckit::Log::debug<LibMir>() << "ConditionT<std::string>::eval("
                                    << ",key=" << key_ << ") failed " << err << std::endl;
        GRIB_ERROR(err, key_);
    }

    return value_ == buffer;
}

class ConditionOR : public Condition {
    const Condition* left_;
    const Condition* right_;
    virtual bool eval(grib_handle* h) const { return left_->eval(h) || right_->eval(h); }

public:
    ConditionOR(const Condition* left, const Condition* right) : left_(left), right_(right) {}
};

class ConditionAND : public Condition {
    const Condition* left_;
    const Condition* right_;
    virtual bool eval(grib_handle* h) const { return left_->eval(h) && right_->eval(h); }

public:
    ConditionAND(const Condition* left, const Condition* right) : left_(left), right_(right) {}
};

class ConditionNOT : public Condition {
    const Condition* c_;
    virtual bool eval(grib_handle* h) const { return !c_->eval(h); }

public:
    ConditionNOT(const Condition* c) : c_(c) {}
};

template <class T>
static Condition* is(const char* key, const T& value) {
    return new ConditionT<T>(key, value);
}

static Condition* is(const char* key, const char* value) {
    return new ConditionT<std::string>(key, value);
}

/*
static Condition *_and(const Condition *left, const Condition *right) {
    return new ConditionAND(left, right);
}
*/

static Condition* _or(const Condition* left, const Condition* right) {
    return new ConditionOR(left, right);
}

/*
static Condition *_not(const Condition *c) {
    return new ConditionNOT(c);
}
*/


void wrongly_encoded_grib(std::string msg) {
    static bool abortIfWronglyEncodedGRIB = eckit::Resource<bool>("$MIR_ABORT_IF_WRONGLY_ENCODED_GRIB", false);
    if (abortIfWronglyEncodedGRIB) {
        eckit::Log::error() << msg << std::endl;
        throw eckit::UserError(msg);
    }
    else {
        eckit::Log::warning() << msg << std::endl;
    }
}


static const char* get_key(const std::string& name, grib_handle* h) {

    static struct {
        const char* name;
        const char* key;
        const Condition* condition;
    } mappings[] = {
        {"west_east_increment", "iDirectionIncrementInDegrees_fix_for_periodic_regular_grids",
         is("gridType", "regular_ll")},
        {"west_east_increment", "iDirectionIncrementInDegrees", nullptr},
        {"south_north_increment", "jDirectionIncrementInDegrees", nullptr},

        {"west", "longitudeOfFirstGridPointInDegrees", nullptr},
        {"east", "longitudeOfLastGridPointInDegrees_fix_for_global_reduced_grids", is("gridType", "reduced_gg")},
        {"east", "longitudeOfLastGridPointInDegrees", nullptr},

        {"north", "latitudeOfFirstGridPointInDegrees", is("scanningMode", 0L)},
        {"south", "latitudeOfLastGridPointInDegrees", is("scanningMode", 0L)},

        {"north", "latitudeOfLastGridPointInDegrees", is("jScansPositively", 1L)},
        {"south", "latitudeOfFirstGridPointInDegrees", is("jScansPositively", 1L)},
        {"north", "latitudeOfFirstGridPointInDegrees", nullptr},
        {"south", "latitudeOfLastGridPointInDegrees", nullptr},

        {"truncation", "pentagonalResolutionParameterJ", nullptr},  // Assumes triangular truncation
        {"accuracy", "bitsPerValue", nullptr},
        {"packing", "packingType", nullptr},

        {"south_pole_latitude", "latitudeOfSouthernPoleInDegrees", nullptr},
        {"south_pole_longitude", "longitudeOfSouthernPoleInDegrees", nullptr},
        {"south_pole_rotation_angle", "angleOfRotationInDegrees", nullptr},

        // This will be just called for has()
        {
            "gridded",
            "Nx",
            _or(_or(is("gridType", "polar_stereographic"), is("gridType", "lambert_azimuthal_equal_area")),
                is("gridType", "lambert")),
        },
        {
            "gridded",
            "Ni",
            is("gridType", "triangular_grid"),
        },
        {
            "gridded",
            "numberOfGridInReference",
            is("gridType", "unstructured_grid"),
        },  // numberOfGridInReference is just dummy

        {"gridded", "numberOfPointsAlongAMeridian", nullptr},  // Is that always true?

        {"gridname", "gridName", nullptr},
        {"angularPrecision", "angularPrecisionInDegrees", nullptr},

        {"spectral", "pentagonalResolutionParameterJ", nullptr},

        /// FIXME: Find something that does no clash
        {"reduced", "numberOfParallelsBetweenAPoleAndTheEquator", is("isOctahedral", 0L)},
        {"regular", "N", is("gridType", "regular_gg")},
        {"octahedral", "numberOfParallelsBetweenAPoleAndTheEquator", is("isOctahedral", 1L)},

        /// TODO: is that a good idea?
        {"param", "paramId", nullptr},
        {"statistics", "", nullptr},  // (avoid ecCodes error "statistics: Function not yet implemented")

        {nullptr, nullptr, nullptr},
    };

    const char* key = name.c_str();
    size_t i        = 0;
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

struct Processing {
    Processing()          = default;
    virtual ~Processing() = default;

    Processing(const Processing&) = delete;
    void operator=(const Processing&) = delete;

    virtual bool eval(grib_handle*, long&) const { NOTIMP; }
    virtual bool eval(grib_handle*, double&) const { NOTIMP; }
    virtual bool eval(grib_handle*, std::vector<double>&) const { NOTIMP; }
};

template <typename T>
struct ProcessingT : Processing {
    using fun_t = std::function<bool(grib_handle*, T&)>;
    fun_t fun_;
    ProcessingT(fun_t&& fun) : fun_(fun) {}
    bool eval(grib_handle* h, T& v) const override { return fun_(h, v); }
};

static ProcessingT<long>* is_wind_component_uv() {
    return new ProcessingT<long>([=](grib_handle* h, long& value) {
        long paramId = 0;
        GRIB_CALL(grib_get_long(h, "paramId", &paramId));
        static const util::Wind::Defaults def;
        long ind = paramId % 1000;
        value    = (ind == def.u ? 1 : ind == def.v ? 2 : 0);
        return value;
    });
}

static ProcessingT<long>* is_wind_component_vod() {
    return new ProcessingT<long>([=](grib_handle* h, long& value) {
        long paramId = 0;
        GRIB_CALL(grib_get_long(h, "paramId", &paramId));
        static const util::Wind::Defaults def;
        long ind = paramId % 1000;
        value    = (ind == def.vo ? 1 : ind == def.d ? 2 : 0);
        return value;
    });
}

static ProcessingT<double>* inverse(const char* key) {
    return new ProcessingT<double>([=](grib_handle* h, double& value) {
        double inv = 0;
        GRIB_CALL(grib_get_double(h, key, &inv));
        ASSERT(!eckit::types::is_approximately_equal<double>(inv, 0));
        value = 1. / inv;
        return true;
    });
}

static ProcessingT<double>* longitudeOfLastGridPointInDegrees_fix_for_global_reduced_grids() {
    return new ProcessingT<double>([](grib_handle* h, double& Lon2) {
        Lon2 = 0;
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
                        msgs << "GribInput: wrongly encoded longitudeOfLastGridPointInDegrees:"
                             << "\n"
                                "encoded:  "
                             << Lon2
                             << "\n"
                                "expected: "
                             << double(Lon2_expected) << " (" << Lon2_expected << " +- " << eps << ")";

                        wrongly_encoded_grib(msgs.str());

                        Lon2 = Lon2_expected;
                    }
                }
            }
        }

        return true;
    });
};

static ProcessingT<double>* iDirectionIncrementInDegrees_fix_for_periodic_regular_grids() {
    return new ProcessingT<double>([](grib_handle* h, double& we) {
        long iScansPositively = 0L;
        GRIB_CALL(grib_get_long(h, "iScansPositively", &iScansPositively));
        ASSERT(iScansPositively == 1L);

        ASSERT(GRIB_CALL(grib_get_double(h, "iDirectionIncrementInDegrees", &we)));
        ASSERT(we > 0.);

        double Lon1 = 0.;
        double Lon2 = 0.;
        long Ni     = 0;
        GRIB_CALL(grib_get_double(h, "longitudeOfFirstGridPointInDegrees", &Lon1));
        GRIB_CALL(grib_get_double(h, "longitudeOfLastGridPointInDegrees", &Lon2));
        GRIB_CALL(grib_get_long(h, "Ni", &Ni));
        ASSERT(Ni > 0);

        Lon2 = LongitudeDouble(Lon2).normalise(Lon1).value();
        ASSERT(Lon2 >= Lon1);

        // angles are within +-1/2 precision, so (Lon2 - Lon1 + we) uses factor 3*1/2
        double eps = 0.;
        std::unique_ptr<Processing> precision_in_degrees(inverse("angularPrecision"));
        ASSERT(precision_in_degrees->eval(h, eps));
        eps *= 1.5;

        double globe = LongitudeDouble::GLOBE.value();
        if (eckit::types::is_approximately_equal(Lon2 - Lon1 + we, globe, eps)) {
            we = globe / Ni;
        }
        else if (!eckit::types::is_approximately_equal(Lon1 + (Ni - 1) * we, Lon2, eps)) {

            // TODO refactor, not really specific to "periodic regular grids", but useful
            std::ostringstream msgs;
            msgs.precision(32);
            msgs << "GribInput: wrongly encoded iDirectionIncrementInDegrees:"
                    "\n"
                    "encoded: "
                 << we
                 << "\n"
                    "Ni: "
                 << Ni
                 << "\n"
                    "longitudeOfFirstGridPointInDegree: "
                 << Lon1
                 << "\n"
                    "longitudeOfLastGridPointInDegrees: "
                 << Lon2;

            wrongly_encoded_grib(msgs.str());
        }

        return true;
    });
};

static ProcessingT<double>* divide(const char* key, double denominator) {
    ASSERT(!eckit::types::is_approximately_equal<double>(denominator, 0));
    return new ProcessingT<double>([=](grib_handle* h, double& value) {
        GRIB_CALL(grib_get_double(h, key, &value));
        value /= denominator;
        return true;
    });
}

static ProcessingT<std::vector<double>>* vector_double(std::initializer_list<std::string> keys) {
    const std::vector<std::string> keys_(keys);
    return new ProcessingT<std::vector<double>>([=](grib_handle* h, std::vector<double>& values) {
        ASSERT(keys.size());

        values.assign(keys_.size(), 0);
        size_t i = 0;
        for (auto& key : keys_) {
            if (!grib_is_defined(h, key.c_str())) {
                return false;
            }
            GRIB_CALL(grib_get_double(h, key.c_str(), &values[i++]));
        }
        return true;
    });
}

template <typename T>
static bool get_value(const std::string& name, grib_handle* h, T& value) {

    static struct {
        const char* name;
        const Processing* processing;
        const Condition* condition;
    } processings[] = {

        {"angularPrecisionInDegrees", inverse("angularPrecision"), nullptr},
        {"longitudeOfLastGridPointInDegrees_fix_for_global_reduced_grids",
         longitudeOfLastGridPointInDegrees_fix_for_global_reduced_grids(), nullptr},
        {"iDirectionIncrementInDegrees_fix_for_periodic_regular_grids",
         iDirectionIncrementInDegrees_fix_for_periodic_regular_grids(), nullptr},

        {"xDirectionGridLengthInMetres", divide("xDirectionGridLengthInMillimetres", 1000.), nullptr},
        {"yDirectionGridLengthInMetres", divide("yDirectionGridLengthInMillimetres", 1000.), nullptr},
        {"standardParallelInDegrees", divide("standardParallelInMicrodegrees", 1000000.), nullptr},
        {"centralLongitudeInDegrees", divide("centralLongitudeInMicrodegrees", 1000000.), nullptr},

        {"grid", vector_double({"iDirectionIncrementInDegrees", "jDirectionIncrementInDegrees"}),
         _or(is("gridType", "regular_ll"), is("gridType", "rotated_ll"))},
        {"grid", vector_double({"xDirectionGridLengthInMetres", "yDirectionGridLengthInMetres"}),
         is("gridType", "lambert_azimuthal_equal_area")},
        {"grid", vector_double({"DxInMetres", "DyInMetres"}), is("gridType", "lambert")},

        {"rotation", vector_double({"latitudeOfSouthernPoleInDegrees", "longitudeOfSouthernPoleInDegrees"}),
         _or(_or(_or(is("gridType", "rotated_ll"), is("gridType", "rotated_gg")), is("gridType", "rotated_sh")),
             is("gridType", "reduced_rotated_gg"))},

        {"is_wind_component_uv", is_wind_component_uv(), nullptr},
        {"is_wind_component_vod", is_wind_component_vod(), nullptr},

        {nullptr, nullptr, nullptr}};

    size_t i = 0;
    while (processings[i].name) {
        if (name == processings[i].name) {
            if (processings[i].condition == nullptr || processings[i].condition->eval(h)) {
                ASSERT(processings[i].processing);
                return processings[i].processing->eval(h, value);
            }
        }
        i++;
    }

    return false;
}

void get_unique_missing_value(const MIRValuesVector& values, double& missing) {
    ASSERT(values.size());

    // check if it's unique, otherwise a high then a low value
    if (std::find(values.begin(), values.end(), missing) == values.end()) {
        return;
    }

    auto mm = std::minmax_element(values.begin(), values.end());
    missing = *(mm.second) + 1.;
    if (missing == missing) {
        return;
    }

    missing = *(mm.first) - 1.;
    if (missing == missing) {
        return;
    }

    throw eckit::SeriousBug("GribInput: get_unique_missing_value: failed to get a unique missing value.");
}


size_t fix_pl_array_zeros(std::vector<long>& pl) {
    wrongly_encoded_grib("GribInput: wrongly encoded pl array contains zeros");

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


}  // namespace


GribInput::GribInput() : cache_(*this), grib_(nullptr) {}


GribInput::~GribInput() {
    handle(nullptr);  // Will delete handle
}


const param::MIRParametrisation& GribInput::parametrisation(size_t which) const {
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

    long earthIsOblate;
    if (GRIB_GET(grib_get_long(grib_, "earthIsOblate", &earthIsOblate))) {
        if (earthIsOblate != 0) {
            throw eckit::UserError("GribInput: GRIB earthIsOblate!=0 not supported");
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

    // Ensure missingValue is unique, so values are not wrongly "missing"
    long numberOfMissingValues = 0;
    if (grib_get_long(grib_, "numberOfMissingValues", &numberOfMissingValues) == GRIB_SUCCESS &&
        numberOfMissingValues == 0) {
        get_unique_missing_value(values, missing);
    }

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
                eckit::Log::debug<LibMir>()
                    << "GribInput::field(): introducing missing values (setting bitmap)." << std::endl;
                missingValuesPresent = 1;
                get_unique_missing_value(values, missing);
            }

            // pl array: insert entries in place of zeros
            std::vector<long> pl_fixed = pl;
            size_t new_values          = fix_pl_array_zeros(pl_fixed);
            ASSERT(new_values > 0);


            // values array: copy values row by row, and when a fixed (0) entry is found, insert missing values
            eckit::Log::debug<LibMir>() << "GribInput::field(): correcting values array with " << new_values
                                        << " new missing values." << std::endl;

            MIRValuesVector values_extended;
            values_extended.reserve(count + new_values);

            ASSERT(pl.size() == pl_fixed.size());
            size_t i = 0;
            for (auto p1 = pl.begin(), p2 = pl_fixed.begin(); p1 != pl.end(); ++p1, ++p2) {
                if (*p1 == 0) {
                    ASSERT(*p2 > 0);
                    size_t Ni = size_t(*p2);

                    values_extended.insert(values_extended.end(), Ni, missing);
                }
                else {
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


grib_handle* GribInput::gribHandle(size_t which) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    ASSERT(which == 0);
    return grib_;
}


bool GribInput::has(const std::string& name) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    ASSERT(grib_);
    const char* key = get_key(name, grib_);
    if (std::string(key).empty()) {
        return false;
    }

    bool ok = grib_is_defined(grib_, key);

    // eckit::Log::debug<LibMir>() << "GribInput::has(" << name << ",key=" << key << ") " << (ok ? "yes" : "no") <<
    // std::endl;
    return ok;
}


bool GribInput::get(const std::string& name, bool& value) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    ASSERT(grib_);
    const char* key = get_key(name, grib_);
    if (std::string(key).empty()) {
        return false;
    }

    long temp = GRIB_MISSING_LONG;
    int err   = grib_get_long(grib_, key, &temp);

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
    const char* key = get_key(name, grib_);
    if (std::string(key).empty()) {
        return false;
    }

    int err = grib_get_long(grib_, key, &value);

    // FIXME: make sure that 'value' is not set if GRIB_MISSING_LONG
    if (err == GRIB_NOT_FOUND || value == GRIB_MISSING_LONG) {
        return get_value(key, grib_, value) || FieldParametrisation::get(name, value);
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
    const char* key = get_key(name, grib_);
    if (std::string(key).empty()) {
        return false;
    }

    int err = grib_get_double(grib_, key, &value);

    // FIXME: make sure that 'value' is not set if GRIB_MISSING_DOUBLE
    if (err == GRIB_NOT_FOUND || value == GRIB_MISSING_DOUBLE) {
        return get_value(key, grib_, value) || FieldParametrisation::get(name, value);
    }

    if (err) {
        // eckit::Log::debug<LibMir>() << "grib_get_double(" << name << ",key=" << key << ") failed " << err <<
        // std::endl;
        GRIB_ERROR(err, key);
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
    const char* key = get_key(name, grib_);
    if (std::string(key).empty()) {
        return false;
    }

    size_t count = 0;
    int err      = grib_get_size(grib_, key, &count);

    if (err == GRIB_NOT_FOUND) {
        return FieldParametrisation::get(name, value);
    }

    if (err) {
        eckit::Log::debug<LibMir>() << "grib_get_long_array(" << name << ",key=" << key << ") failed " << err
                                    << " count=" << count << std::endl;
        GRIB_ERROR(err, key);
    }

    size_t size = count;

    value.resize(count);

    GRIB_CALL(grib_get_long_array(grib_, key, &value[0], &size));
    ASSERT(count == size);

    ASSERT(value.size());

    // eckit::Log::debug<LibMir>() << "grib_get_long_array(" << name << ",key=" << key << ") size=" << value.size() <<
    // std::endl;
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
    const char* key = get_key(name, grib_);
    if (std::string(key).empty()) {
        return false;
    }

    char buffer[10240];
    size_t size = sizeof(buffer);
    int err     = grib_get_string(grib_, key, buffer, &size);

    if (err == GRIB_NOT_FOUND) {
        return FieldParametrisation::get(name, value);
    }

    if (err) {
        // eckit::Log::debug<LibMir>() << "grib_get_string(" << name << ",key=" << key << ") failed " << err <<
        // std::endl;
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
    const char* key = get_key(name, grib_);
    if (std::string(key).empty()) {
        return false;
    }

    if (get_value(key, grib_, value)) {
        return true;
    }

    size_t count = 0;
    int err      = grib_get_size(grib_, key, &count);

    if (err == GRIB_NOT_FOUND) {
        return FieldParametrisation::get(name, value);
    }

    if (err) {
        // eckit::Log::debug<LibMir>() << "grib_get_double_array(" << name << ",key=" << key << ") failed " << err << "
        // count=" << count << std::endl;
        GRIB_ERROR(err, key);
    }

    size_t size = count;

    value.resize(count);

    GRIB_CALL(grib_get_double_array(grib_, key, &value[0], &size));
    ASSERT(count == size);

    ASSERT(value.size());

    // eckit::Log::debug<LibMir>() << "grib_get_double_array(" << name << ",key=" << key << ") size=" << value.size() <<
    // std::endl;


    return true;
}

bool GribInput::get(const std::string&, std::vector<std::string>&) const {
    NOTIMP;
}


bool GribInput::handle(grib_handle* h) {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    FieldParametrisation::reset();
    cache_.reset();

    grib_handle_delete(grib_);
    grib_ = h;

    if (h != nullptr) {
        long value = 0;
        GRIB_CALL(grib_get_long(h, "7777", &value));
        return true;
    }

    return false;
}


void GribInput::auxilaryValues(const std::string& path, std::vector<double>& values) const {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    eckit::AutoStdFile f(path);

    int e;
    grib_handle* h = nullptr;

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
    }
    catch (...) {
        grib_handle_delete(h);
        throw;
    }
}


void GribInput::setAuxiliaryInformation(const std::string& yaml) {
    eckit::AutoLock<eckit::Mutex> lock(mutex_);

    eckit::ValueMap keyValue = eckit::YAMLParser::decodeString(yaml);
    for (const auto& kv : keyValue) {
        if (kv.first == "latitudes") {
            eckit::Log::debug<LibMir>() << "Loading auxilary file '" << kv.second << "'" << std::endl;
            auxilaryValues(kv.second, latitudes_);
        }
        else if (kv.first == "longitudes") {
            eckit::Log::debug<LibMir>() << "Loading auxilary file '" << kv.second << "'" << std::endl;
            auxilaryValues(kv.second, longitudes_);
        }
    }
}


size_t GribInput::dimensions() const {
    // This will be one probably for a long time
    return 1;
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

    grib_keys_iterator* keys =
        grib_keys_iterator_new(grib_, GRIB_KEYS_ITERATOR_ALL_KEYS, gribToRequestNamespace.c_str());
    ASSERT(keys);

    try {
        const char* sep = "";
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
        int err     = grib_get_size(grib_, "freeFormData", &size);

        if (err == 0) {
            eckit::Buffer buffer(size);
            char* b = buffer;

            GRIB_CALL(grib_get_bytes(grib_, "freeFormData", (unsigned char*)b, &size));
            ASSERT(size == buffer.size());

            eckit::MemoryHandle h(buffer);
            eckit::HandleStream in(h);
            int n;
            in >> n;  // Number of requests
            ASSERT(n == 1);
            std::string verb;
            in >> verb;

            in >> n;
            for (int i = 0; i < n; i++) {
                std::string param;
                in >> param;
                out << sep << param;
                sep               = ",";
                const char* slash = "=";
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
    }
    catch (...) {
        if (keys) {
            grib_keys_iterator_delete(keys);
        }
        throw;
    }
}


}  // namespace input
}  // namespace mir
