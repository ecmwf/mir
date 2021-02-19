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
#include <memory>
#include <numeric>
#include <sstream>

#include "eckit/config/Resource.h"
#include "eckit/io/Buffer.h"
#include "eckit/io/MemoryHandle.h"
#include "eckit/io/StdFile.h"
#include "eckit/parser/YAMLParser.h"
#include "eckit/serialisation/HandleStream.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/types/Fraction.h"

#include "mir/data/MIRField.h"
#include "mir/input/GribFixes.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"
#include "mir/util/LongitudeDouble.h"
#include "mir/util/Wind.h"


namespace mir {
namespace input {


namespace {


class Condition {
public:
    Condition() = default;

    Condition(const Condition&) = delete;
    Condition& operator=(const Condition&) = delete;

    virtual ~Condition()                  = default;
    virtual bool eval(grib_handle*) const = 0;
};


template <class T>
class ConditionT : public Condition {
    const char* key_;
    T value_;
    bool eval(grib_handle*) const override;

public:
    ConditionT(const char* key, const T& value) : key_(key), value_(value) {}
};


template <>
bool ConditionT<long>::eval(grib_handle* h) const {
    long value;
    ASSERT(h);
    int err = codes_get_long(h, key_, &value);

    if (err == CODES_NOT_FOUND) {
        return false;
    }

    if (err != 0) {
        // Log::debug() << "ConditionT<long>::eval(" << ",key=" << key_ << ") failed " << err <<
        // std::endl;
        GRIB_ERROR(err, key_);
    }

    return value_ == value;
}


template <>
bool ConditionT<double>::eval(grib_handle* h) const {
    double value;
    ASSERT(h);
    int err = codes_get_double(h, key_, &value);

    if (err == CODES_NOT_FOUND) {
        return false;
    }

    if (err != 0) {
        // Log::debug() << "ConditionT<double>::eval(" << ",key=" << key_ << ") failed " << err <<
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
    int err = codes_get_string(h, key_, buffer, &size);

    if (err == CODES_NOT_FOUND) {
        return false;
    }

    if (err != 0) {
        Log::debug() << "ConditionT<std::string>::eval("
                     << ",key=" << key_ << ") failed " << err << std::endl;
        GRIB_ERROR(err, key_);
    }

    return value_ == buffer;
}


class ConditionOR : public Condition {
    const Condition* left_;
    const Condition* right_;
    bool eval(grib_handle* h) const override { return left_->eval(h) || right_->eval(h); }
    ~ConditionOR() {
        delete right_;
        delete left_;
    }

public:
    ConditionOR(const Condition* left, const Condition* right) : left_(left), right_(right) {}
};


/*
class ConditionAND : public Condition {
    const Condition* left_;
    const Condition* right_;
     bool eval(grib_handle* h) const override  { return left_->eval(h) && right_->eval(h); }

public:
    ConditionAND(const Condition* left, const Condition* right) : left_(left), right_(right) {}
};
*/


/*
class ConditionNOT : public Condition {
    const Condition* c_;
     bool eval(grib_handle* h) const  override { return !c_->eval(h); }

public:
    ConditionNOT(const Condition* c) : c_(c) {}
};
*/


}  // namespace


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


void wrongly_encoded_grib(const std::string& msg) {
    static bool abortIfWronglyEncodedGRIB = eckit::Resource<bool>("$MIR_ABORT_IF_WRONGLY_ENCODED_GRIB", false);

    if (abortIfWronglyEncodedGRIB) {
        Log::error() << msg << std::endl;
        throw exception::UserError(msg);
    }

    Log::warning() << msg << std::endl;
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

        {"south_pole_latitude", "latitudeOfSouthernPoleInDegrees", nullptr},
        {"south_pole_longitude", "longitudeOfSouthernPoleInDegrees", nullptr},
        {"south_pole_rotation_angle", "angleOfRotationInDegrees", nullptr},

        {"proj", "projTargetString", nullptr},
        {"projSource", "projSourceString", nullptr},

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
            "numberOfGridInReference" /*just a dummy*/,
            is("gridType", "unstructured_grid"),
        },
        {"gridded", "numberOfPointsAlongAMeridian", nullptr},  // Is that always true?
        {"gridded_regular_ll", "Ni", _or(is("gridType", "regular_ll"), is("gridType", "rotated_ll"))},
        {"gridded_named", "gridName", nullptr},

        {"grid", "gridName",
         _or(_or(_or(_or(is("gridType", "regular_gg"), is("gridType", "reduced_gg")), is("gridType", "rotated_gg")),
                 is("gridType", "reduced_rotated_gg")),
             is("gridType", "unstructured_grid"))},

        {"spectral", "pentagonalResolutionParameterJ", nullptr},

        {"uid", "uuidOfHGrid", is("gridType", "unstructured_grid")},

        /// FIXME: Find something that does no clash
        {"reduced", "numberOfParallelsBetweenAPoleAndTheEquator", is("isOctahedral", 0L)},
        {"regular", "N", is("gridType", "regular_gg")},
        {"octahedral", "numberOfParallelsBetweenAPoleAndTheEquator", is("isOctahedral", 1L)},

        /// TODO: is that a good idea?
        {"param", "paramId", nullptr},
        {"statistics", "", nullptr},  // (avoid ecCodes error "statistics: Function not yet implemented")

        {nullptr, nullptr, nullptr},
    };

    for (size_t i = 0; mappings[i].name != nullptr; ++i) {
        if (name == mappings[i].name) {
            if (mappings[i].condition == nullptr || mappings[i].condition->eval(h)) {
                return mappings[i].key;
            }
        }
    }

    auto key = name.c_str();
    return key;
}


template <typename T>
struct ProcessingT {
    using fun_t = std::function<bool(grib_handle*, T&)>;
    fun_t fun_;
    ProcessingT(fun_t&& fun) : fun_(fun) {}
    ProcessingT(const ProcessingT&) = delete;
    void operator=(const ProcessingT&) = delete;
    bool eval(grib_handle* h, T& v) const { return fun_(h, v); }
};


static ProcessingT<long>* is_wind_component_uv() {
    return new ProcessingT<long>([](grib_handle* h, long& value) {
        long paramId = 0;
        GRIB_CALL(codes_get_long(h, "paramId", &paramId));
        static const util::Wind::Defaults def;
        long ind = paramId % 1000;
        value    = (ind == def.u ? 1 : ind == def.v ? 2 : 0);
        return value;
    });
}


static ProcessingT<long>* is_wind_component_vod() {
    return new ProcessingT<long>([](grib_handle* h, long& value) {
        long paramId = 0;
        GRIB_CALL(codes_get_long(h, "paramId", &paramId));
        static const util::Wind::Defaults def;
        long ind = paramId % 1000;
        value    = (ind == def.vo ? 1 : ind == def.d ? 2 : 0);
        return value;
    });
}


static ProcessingT<double>* angular_precision() {
    return new ProcessingT<double>([](grib_handle* h, double& value) {
        auto well_defined = [](grib_handle* h, const char* key) -> bool {
            long dummy = 0;
            int err    = 0;
            return (codes_is_defined(h, key) != 0) && (codes_is_missing(h, key, &err) == 0) && (err == CODES_SUCCESS) &&
                   (codes_get_long(h, key, &dummy) == CODES_SUCCESS) && (dummy != 0);
        };

        if (well_defined(h, "basicAngleOfTheInitialProductionDomain") && well_defined(h, "subdivisionsOfBasicAngle")) {
            value = 0.;
            return true;
        }

        long angleSubdivisions = 0;
        GRIB_CALL(codes_get_long(h, "angleSubdivisions", &angleSubdivisions));

        value = angleSubdivisions > 0 ? 1. / double(angleSubdivisions) : 0.;
        return true;
    });
}


static ProcessingT<double>* longitudeOfLastGridPointInDegrees_fix_for_global_reduced_grids() {
    return new ProcessingT<double>([](grib_handle* h, double& Lon2) {
        Lon2 = 0;
        GRIB_CALL(codes_get_double(h, "longitudeOfLastGridPointInDegrees", &Lon2));

        if (codes_is_defined(h, "pl") != 0) {

            double Lon1 = 0;
            GRIB_CALL(codes_get_double(h, "longitudeOfFirstGridPointInDegrees", &Lon1));

            if (eckit::types::is_approximately_equal<double>(Lon1, 0)) {

                // get pl array maximum and sum
                // if sum equals values size the grid must be global
                size_t plSize = 0;
                GRIB_CALL(codes_get_size(h, "pl", &plSize));
                ASSERT(plSize);

                std::vector<long> pl(plSize, 0);
                size_t plSizeAsRead = plSize;
                GRIB_CALL(codes_get_long_array(h, "pl", pl.data(), &plSizeAsRead));
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
                GRIB_CALL(codes_get_size(h, "values", &valuesSize));

                if (size_t(plSum) == valuesSize) {

                    double eps = 0.;
                    std::unique_ptr<ProcessingT<double>> precision_in_degrees(angular_precision());
                    ASSERT(precision_in_degrees->eval(h, eps));

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
        GRIB_CALL(codes_get_long(h, "iScansPositively", &iScansPositively));
        ASSERT(iScansPositively == 1L);

        ASSERT(GRIB_CALL(codes_get_double(h, "iDirectionIncrementInDegrees", &we)));
        ASSERT(we > 0.);

        double Lon1 = 0.;
        double Lon2 = 0.;
        long Ni     = 0;
        GRIB_CALL(codes_get_double(h, "longitudeOfFirstGridPointInDegrees", &Lon1));
        GRIB_CALL(codes_get_double(h, "longitudeOfLastGridPointInDegrees", &Lon2));
        GRIB_CALL(codes_get_long(h, "Ni", &Ni));
        ASSERT(Ni > 0);

        Lon2 = LongitudeDouble(Lon2).normalise(Lon1).value();
        ASSERT(Lon2 >= Lon1);

        // angles are within +-1/2 precision, so (Lon2 - Lon1 + we) uses factor 3*1/2
        double eps = 0.;
        std::unique_ptr<ProcessingT<double>> precision_in_degrees(angular_precision());
        ASSERT(precision_in_degrees->eval(h, eps));
        eps *= 1.5;

        auto Nid     = double(Ni);
        double globe = LongitudeDouble::GLOBE.value();
        if (eckit::types::is_approximately_equal(Lon2 - Lon1 + we, globe, eps)) {
            we = globe / Nid;
        }
        else if (!eckit::types::is_approximately_equal(Lon1 + (Nid - 1) * we, Lon2, eps)) {

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


static ProcessingT<std::vector<double>>* vector_double(std::initializer_list<std::string> keys) {
    const std::vector<std::string> keys_(keys);
    return new ProcessingT<std::vector<double>>([=](grib_handle* h, std::vector<double>& values) {
        ASSERT(keys.size());

        values.assign(keys_.size(), 0);
        size_t i = 0;
        for (auto& key : keys_) {
            if (codes_is_defined(h, key.c_str()) == 0) {
                return false;
            }
            GRIB_CALL(codes_get_double(h, key.c_str(), &values[i++]));
        }
        return true;
    });
}


static ProcessingT<std::string>* packing() {
    return new ProcessingT<std::string>([](grib_handle* h, std::string& value) {
        auto get = [](grib_handle* h, const char* key) -> std::string {
            if (codes_is_defined(h, key) != 0) {
                char buffer[64];
                size_t size = sizeof(buffer);

                GRIB_CALL(codes_get_string(h, key, buffer, &size));
                ASSERT(size < sizeof(buffer) - 1);

                if (::strcmp(buffer, "MISSING") != 0) {
                    return buffer;
                }
            }
            return "";
        };

        auto packingType = get(h, "packingType");
        for (std::string prefix : {"grid_", "spectral_"}) {
            if (packingType.find(prefix) == 0) {
                value = packingType.substr(prefix.size());
                std::replace(value.begin(), value.end(), '_', '-');
                return true;
            }
        }

        return false;
    });
}


template <typename T, typename P>
static bool get_value(const std::string& name, grib_handle* h, T& value, const P& process) {
    for (size_t i = 0; process[i].name != nullptr; ++i) {
        auto& p = process[i];
        if (name == p.name) {
            if (p.condition == nullptr || p.condition->eval(h)) {
                ASSERT(p.processing);
                return p.processing->eval(h, value);
            }
        }
    }
    return false;
}


namespace {


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

    throw exception::SeriousBug("GribInput: get_unique_missing_value: failed to get a unique missing value.");
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
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    ASSERT(grib_);

    long localDefinitionNumber = 0;
    if (codes_get_long(grib_, "localDefinitionNumber", &localDefinitionNumber) == CODES_SUCCESS) {
        if (localDefinitionNumber == 4) {
            throw exception::UserError("GribInput: GRIB localDefinitionNumber=4 ('ocean') not supported");
        }
    }

    long earthIsOblate = 0;
    if (GRIB_GET(codes_get_long(grib_, "earthIsOblate", &earthIsOblate))) {
        if (earthIsOblate != 0) {
            wrongly_encoded_grib("GribInput: only spherical earth supported (earthIsOblate != 0)");
        }
    }

    size_t count;
    GRIB_CALL(codes_get_size(grib_, "values", &count));

    size_t size = count;
    MIRValuesVector values(count);
    GRIB_CALL(codes_get_double_array(grib_, "values", &values[0], &size));
    ASSERT(count == size);

    long missingValuesPresent;
    GRIB_CALL(codes_get_long(grib_, "missingValuesPresent", &missingValuesPresent));

    double missing;
    GRIB_CALL(codes_get_double(grib_, "missingValue", &missing));

    // Ensure missingValue is unique, so values are not wrongly "missing"
    long numberOfMissingValues = 0;
    if (codes_get_long(grib_, "numberOfMissingValues", &numberOfMissingValues) == CODES_SUCCESS &&
        numberOfMissingValues == 0) {
        get_unique_missing_value(values, missing);
    }

    // If grib has a 0-containing pl array, add missing values in their place
    if (has("pl")) {
        size_t count_pl = 0;
        GRIB_CALL(codes_get_size(grib_, "pl", &count_pl));
        ASSERT(count_pl);

        std::vector<long> pl(count_pl, 0);
        size = count_pl;
        GRIB_CALL(codes_get_long_array(grib_, "pl", pl.data(), &size));
        ASSERT(count_pl == size);

        // NOTE: this fix ties with the method get(const std::string &name, std::vector<long> &value)
        if (std::find(pl.rbegin(), pl.rend(), 0) != pl.rend()) {

            // if there are no missing values yet, set them
            if (missingValuesPresent == 0) {
                Log::debug() << "GribInput::field(): introducing missing values (setting bitmap)." << std::endl;
                missingValuesPresent = 1;
                get_unique_missing_value(values, missing);
            }

            // pl array: insert entries in place of zeros
            std::vector<long> pl_fixed = pl;
            size_t new_values          = fix_pl_array_zeros(pl_fixed);
            ASSERT(new_values > 0);


            // values array: copy values row by row, and when a fixed (0) entry is found, insert missing values
            Log::debug() << "GribInput::field(): correcting values array with " << new_values << " new missing values."
                         << std::endl;

            MIRValuesVector values_extended;
            values_extended.reserve(count + new_values);

            ASSERT(pl.size() == pl_fixed.size());
            size_t i = 0;
            for (auto p1 = pl.begin(), p2 = pl_fixed.begin(); p1 != pl.end(); ++p1, ++p2) {
                if (*p1 == 0) {
                    ASSERT(*p2 > 0);
                    auto Ni = size_t(*p2);
                    values_extended.insert(values_extended.end(), Ni, missing);
                }
                else {
                    auto Ni = size_t(*p1);
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

    data::MIRField field(cache_, missingValuesPresent != 0, missing);

    long scanningMode = 0;
    if (codes_get_long(grib_, "scanningMode", &scanningMode) == CODES_SUCCESS && scanningMode != 0) {
        field.representation()->reorder(scanningMode, values);
    }

    field.update(values, 0);
    field.validate();

    return field;
}


grib_handle* GribInput::gribHandle(size_t which) const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    ASSERT(which == 0);
    return grib_;
}


bool GribInput::has(const std::string& name) const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    ASSERT(grib_);
    const char* key = get_key(name, grib_);
    if (std::string(key).empty()) {
        return false;
    }

    bool ok = codes_is_defined(grib_, key) != 0;

    // Log::debug() << "GribInput::has(" << name << ",key=" << key << ") " << (ok ? "yes" : "no") <<
    // std::endl;
    return ok;
}


bool GribInput::get(const std::string& name, bool& value) const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    ASSERT(grib_);
    const char* key = get_key(name, grib_);
    if (std::string(key).empty()) {
        return false;
    }

    // FIXME: make sure that 'temp' is not set if CODES_MISSING_LONG
    long temp = CODES_MISSING_LONG;
    int err   = codes_get_long(grib_, key, &temp);
    if (err == CODES_NOT_FOUND || codes_is_missing(grib_, key, &err) != 0) {
        return FieldParametrisation::get(name, value);
    }

    if (err != 0) {
        // Log::debug() << "codes_get_bool(" << name << ",key=" << key << ") failed " << err <<
        // std::endl;
        GRIB_ERROR(err, key);
    }

    value = temp != 0;

    // Log::debug() << "codes_get_bool(" << name << ",key=" << key << ") " << value << std::endl;
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
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    ASSERT(grib_);
    std::string key = get_key(name, grib_);
    if (key.empty()) {
        return false;
    }

    std::string packing;
    if (key == "bitsPerValue" && get("packing", packing) && packing == "ieee") {
        // GRIB2 Section 5 Code Table 7
        // NOTE: has to be done here as GRIBs packingType=grid_ieee ignores bitsPerValue (usually 0?)
        long precision = 0;
        GRIB_CALL(codes_get_long(grib_, "precision", &precision));
        value = precision == 1 ? 32 : precision == 2 ? 64 : precision == 3 ? 128 : 0;
        return value != 0;
    }

    // FIXME: make sure that 'value' is not set if CODES_MISSING_LONG
    int err = codes_get_long(grib_, key.c_str(), &value);
    if (err == CODES_NOT_FOUND || codes_is_missing(grib_, key.c_str(), &err) != 0) {
        static struct {
            const char* name;
            const ProcessingT<long>* processing;
            const Condition* condition;
        } process[] = {
            {"is_wind_component_uv", is_wind_component_uv(), nullptr},
            {"is_wind_component_vod", is_wind_component_vod(), nullptr},
            {nullptr, nullptr, nullptr},
        };

        return get_value(key.c_str(), grib_, value, process) || FieldParametrisation::get(name, value);
    }

    if (err != 0) {
        Log::debug() << "codes_get_long(" << name << ",key=" << key << ") failed " << err << std::endl;
        GRIB_ERROR(err, key.c_str());
    }

    // Log::debug() << "codes_get_long(" << name << ",key=" << key << ") " << value << std::endl;
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
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    ASSERT(grib_);
    const char* key = get_key(name, grib_);
    if (std::string(key).empty()) {
        return false;
    }

    // FIXME: make sure that 'value' is not set if CODES_MISSING_DOUBLE
    int err = codes_get_double(grib_, key, &value);
    if (err == CODES_NOT_FOUND || codes_is_missing(grib_, key, &err) != 0) {
        static struct {
            const char* name;
            const ProcessingT<double>* processing;
            const Condition* condition;
        } process[] = {
            {"angular_precision", angular_precision(), nullptr},
            {"longitudeOfLastGridPointInDegrees_fix_for_global_reduced_grids",
             longitudeOfLastGridPointInDegrees_fix_for_global_reduced_grids(), nullptr},
            {"iDirectionIncrementInDegrees_fix_for_periodic_regular_grids",
             iDirectionIncrementInDegrees_fix_for_periodic_regular_grids(), nullptr},
            {nullptr, nullptr, nullptr},
        };

        return get_value(key, grib_, value, process) || FieldParametrisation::get(name, value);
    }

    if (err != 0) {
        // Log::debug() << "codes_get_double(" << name << ",key=" << key << ") failed " << err <<
        // std::endl;
        GRIB_ERROR(err, key);
    }

    // Log::debug() << "codes_get_double(" << name << ",key=" << key << ") " << value << std::endl;
    return true;
}


bool GribInput::get(const std::string&, std::vector<int>&) const {
    NOTIMP;
}


bool GribInput::get(const std::string& name, std::vector<long>& value) const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    ASSERT(grib_);
    const char* key = get_key(name, grib_);
    if (std::string(key).empty()) {
        return false;
    }

    size_t count = 0;
    int err      = codes_get_size(grib_, key, &count);

    if (err == CODES_NOT_FOUND) {
        return FieldParametrisation::get(name, value);
    }

    if (err != 0) {
        Log::debug() << "codes_get_long_array(" << name << ",key=" << key << ") failed " << err << " count=" << count
                     << std::endl;
        GRIB_ERROR(err, key);
    }

    size_t size = count;

    value.resize(count);

    GRIB_CALL(codes_get_long_array(grib_, key, &value[0], &size));
    ASSERT(count == size);

    ASSERT(value.size());

    // Log::debug() << "codes_get_long_array(" << name << ",key=" << key << ") size=" << value.size() <<
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
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    ASSERT(grib_);
    const char* key = get_key(name, grib_);
    if (std::string(key).empty()) {
        return false;
    }

    char buffer[10240];
    size_t size = sizeof(buffer);
    int err     = codes_get_string(grib_, key, buffer, &size);

    if (err == CODES_NOT_FOUND) {
        static struct {
            const char* name;
            const ProcessingT<std::string>* processing;
            const Condition* condition;
        } process[] = {
            {"packing", packing(), nullptr},
            {nullptr, nullptr, nullptr},
        };

        return get_value(key, grib_, value, process) || FieldParametrisation::get(name, value);
    }

    if (err != 0) {
        // Log::debug() << "codes_get_string(" << name << ",key=" << key << ") failed " << err <<
        // std::endl;
        GRIB_ERROR(err, key);
    }

    // Log::info() << err << "  " << size << " " << name << std::endl;

    ASSERT(size < sizeof(buffer) - 1);

    if (::strcmp(buffer, "MISSING") == 0) {
        return false;
    }

    value = buffer;

    // Log::debug() << "codes_get_string(" << name << ",key=" << key << ") " << value << std::endl;

    return true;
}


bool GribInput::get(const std::string& name, std::vector<double>& value) const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    ASSERT(grib_);
    const char* key = get_key(name, grib_);

    // NOTE: MARS client sets 'grid=vector' (deprecated) which needs to be compared against GRIB gridName
    if (std::string(key).empty() || std::string(key) == "gridName") {
        return false;
    }

    static struct {
        const char* name;
        const ProcessingT<std::vector<double>>* processing;
        const Condition* condition;
    } process[] = {
        {"grid", vector_double({"iDirectionIncrementInDegrees", "jDirectionIncrementInDegrees"}),
         _or(is("gridType", "regular_ll"), is("gridType", "rotated_ll"))},
        {"grid", vector_double({"xDirectionGridLengthInMetres", "yDirectionGridLengthInMetres"}),
         is("gridType", "lambert_azimuthal_equal_area")},
        {"grid", vector_double({"DxInMetres", "DyInMetres"}),
         _or(is("gridType", "lambert"), is("gridType", "polar_stereographic"))},
        {"grid", vector_double({"DiInMetres", "DjInMetres"}), is("gridType", "mercator")},
        {"rotation", vector_double({"latitudeOfSouthernPoleInDegrees", "longitudeOfSouthernPoleInDegrees"}),
         _or(_or(_or(is("gridType", "rotated_ll"), is("gridType", "rotated_gg")), is("gridType", "rotated_sh")),
             is("gridType", "reduced_rotated_gg"))},
        {nullptr, nullptr, nullptr},
    };

    if (get_value(key, grib_, value, process)) {
        return true;
    }

    size_t count = 0;
    int err      = codes_get_size(grib_, key, &count);

    if (err == CODES_NOT_FOUND) {
        return FieldParametrisation::get(name, value);
    }

    if (err != 0) {
        // Log::debug() << "codes_get_double_array(" << name << ",key=" << key << ") failed " << err << "
        // count=" << count << std::endl;
        GRIB_ERROR(err, key);
    }

    ASSERT(count > 0);
    size_t size = count;

    value.resize(count);

    GRIB_CALL(codes_get_double_array(grib_, key, &value[0], &size));
    ASSERT(count == size);

    // Log::debug() << "codes_get_double_array(" << name << ",key=" << key << ") size=" << value.size()
    // << std::endl;

    ASSERT(!value.empty());
    return true;
}

bool GribInput::get(const std::string&, std::vector<std::string>&) const {
    NOTIMP;
}


bool GribInput::handle(grib_handle* h) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    FieldParametrisation::reset();
    cache_.reset();

    codes_handle_delete(grib_);
    grib_ = h;

    if (h != nullptr) {
        long value = 0;
        GRIB_CALL(codes_get_long(h, "7777", &value));

        // apply user-defined fixes, if any
        static GribFixes gribFixes;
        if (gribFixes.fix(*this, cache_.cache_)) {
            wrongly_encoded_grib("GribInput: wrongly encoded GRIB (user-defined fixes)");
        }

        return true;
    }

    return false;
}


void GribInput::auxilaryValues(const std::string& path, std::vector<double>& values) const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    eckit::AutoStdFile f(path);

    int e;
    grib_handle* h = nullptr;

    // We cannot use GribFileInput to read these files, because lat/lon files are also
    // has grid_type = triangular_grid, and we will create a loop

    try {
        h = codes_grib_handle_new_from_file(nullptr, f, &e);
        grib_call(e, path.c_str());
        size_t count;
        GRIB_CALL(codes_get_size(h, "values", &count));

        size_t size = count;
        values.resize(count);
        GRIB_CALL(codes_get_double_array(h, "values", &values[0], &size));
        ASSERT(count == size);

        long missingValuesPresent;
        GRIB_CALL(codes_get_long(h, "missingValuesPresent", &missingValuesPresent));
        ASSERT(!missingValuesPresent);

        codes_handle_delete(h);
    }
    catch (...) {
        codes_handle_delete(h);
        throw;
    }
}


void GribInput::setAuxiliaryInformation(const std::string& yaml) {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    eckit::ValueMap keyValue = eckit::YAMLParser::decodeString(yaml);
    for (const auto& kv : keyValue) {
        if (kv.first == "latitudes") {
            Log::debug() << "Loading auxilary file '" << kv.second << "'" << std::endl;
            auxilaryValues(kv.second, latitudes_);
        }
        else if (kv.first == "longitudes") {
            Log::debug() << "Loading auxilary file '" << kv.second << "'" << std::endl;
            auxilaryValues(kv.second, longitudes_);
        }
    }
}


bool GribInput::only(size_t paramId) {
    auto paramIdOnly = long(paramId);

    while (next()) {
        std::lock_guard<std::recursive_mutex> lock(mutex_);

        ASSERT(grib_);

        long paramIdAsLong;
        GRIB_CALL(codes_get_long(grib_, "paramId", &paramIdAsLong));

        if (paramIdOnly == paramIdAsLong) {
            return true;
        }
    }

    return false;
}


size_t GribInput::dimensions() const {
    // This will be one probably for a long time
    return 1;
}


// TODO: some caching, also next() should maybe advance the auxilary files
void GribInput::latitudes(std::vector<double>& values) const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    values.clear();
    values.reserve(latitudes_.size());
    std::copy(latitudes_.begin(), latitudes_.end(), std::back_inserter(values));
}


void GribInput::longitudes(std::vector<double>& values) const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    values.clear();
    values.reserve(longitudes_.size());
    std::copy(longitudes_.begin(), longitudes_.end(), std::back_inserter(values));
}


void GribInput::marsRequest(std::ostream& out) const {
    std::lock_guard<std::recursive_mutex> lock(mutex_);

    ASSERT(grib_);

    static std::string gribToRequestNamespace = eckit::Resource<std::string>("gribToRequestNamespace", "mars");

    auto keys = codes_keys_iterator_new(grib_, CODES_KEYS_ITERATOR_ALL_KEYS, gribToRequestNamespace.c_str());
    ASSERT(keys);

    try {
        const char* sep = "";
        while (codes_keys_iterator_next(keys) != 0) {

            char value[1024];
            size_t size = sizeof(value);
            out << sep << codes_keys_iterator_get_name(keys);
            GRIB_CALL(codes_keys_iterator_get_string(keys, value, &size));
            out << "=" << value;
            sep = ",";
        }
        codes_keys_iterator_delete(keys);
        keys = nullptr;

        size_t size = 0;
        int err     = codes_get_size(grib_, "freeFormData", &size);

        if (err == 0) {
            eckit::Buffer buffer(size);
            char* b = buffer;

            GRIB_CALL(codes_get_bytes(grib_, "freeFormData", (unsigned char*)b, &size));
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


        if (err != CODES_NOT_FOUND) {
            grib_call(err, "freeFormData");
        }
    }
    catch (...) {
        if (keys != nullptr) {
            codes_keys_iterator_delete(keys);
        }
        throw;
    }
}


}  // namespace input
}  // namespace mir
