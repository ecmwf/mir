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
#include <cstring>
#include <functional>
#include <iterator>
#include <memory>
#include <numeric>
#include <ostream>
#include <sstream>
#include <utility>
#include <vector>

#include "eckit/config/Resource.h"
#include "eckit/io/Buffer.h"
#include "eckit/io/MemoryHandle.h"
#include "eckit/serialisation/HandleStream.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/types/Fraction.h"

#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/grib/Config.h"
#include "mir/input/GriddefInput.h"
#include "mir/param/DefaultParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"
#include "mir/util/LongitudeDouble.h"
#include "mir/util/Mutex.h"
#include "mir/util/ValueMap.h"
#include "mir/util/Wind.h"


namespace mir::input {


namespace {


class Condition {
public:
    Condition() = default;

    Condition(const Condition&)            = delete;
    Condition(Condition&&)                 = delete;
    Condition& operator=(const Condition&) = delete;
    Condition& operator=(Condition&&)      = delete;

    virtual ~Condition()                  = default;
    virtual bool eval(grib_handle*) const = 0;
};


template <class T>
class ConditionT : public Condition {
    const char* key_;
    T value_;
    bool eval(grib_handle* /*unused*/) const override;

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
    ~ConditionOR() override {
        delete right_;
        delete left_;
    }

public:
    ConditionOR(const Condition* left, const Condition* right) : left_(left), right_(right) {}

    ConditionOR(const ConditionOR&)            = delete;
    ConditionOR(ConditionOR&&)                 = delete;
    ConditionOR& operator=(const ConditionOR&) = delete;
    ConditionOR& operator=(ConditionOR&&)      = delete;
};


/*
class ConditionAND : public Condition {
    const Condition* left_;
    const Condition* right_;
    bool eval(grib_handle* h) const override  { return left_->eval(h) && right_->eval(h); }
    ~ConditionAND() override {
        delete right_;
        delete left_;
    }

public:
    ConditionAND(const Condition* left, const Condition* right) : left_(left), right_(right) {}

    ConditionAND(const ConditionAND&) = delete;
    ConditionAND( ConditionAND&&) = delete;
    ConditionAND& operator=(const ConditionAND&) = delete;
    ConditionAND& operator=( ConditionAND&&) = delete;
};
*/


/*
class ConditionNOT : public Condition {
    const Condition* c_;
    bool eval(grib_handle* h) const  override { return !c_->eval(h); }
    ~ConditionNOT() override {
        delete c_;
    }

public:
    ConditionNOT(const Condition* c) : c_(c) {}
};
*/


void wrongly_encoded_grib(const std::string& msg) {
    static bool abortIfWronglyEncodedGRIB = eckit::Resource<bool>("$MIR_ABORT_IF_WRONGLY_ENCODED_GRIB", false);

    if (abortIfWronglyEncodedGRIB || forceThrow) {
        Log::error() << msg << std::endl;
        throw exception::UserError(msg);
    }

    Log::warning() << msg << std::endl;
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


static const char* get_key(const std::string& name, grib_handle* h) {
    struct P {
        const std::string name;
        const char* key;
        const std::unique_ptr<const Condition> condition;
        P(const std::string _name, const char* _key, const Condition* _condition = nullptr) :
            name(_name), key(_key), condition(_condition) {}
    };

    static const std::initializer_list<P> mappings{
        {"west_east_increment", "iDirectionIncrementInDegrees_fix_for_periodic_regular_grids",
         is("gridType", "regular_ll")},
        {"west_east_increment", "iDirectionIncrementInDegrees"},
        {"south_north_increment", "jDirectionIncrementInDegrees"},

        {"west", "longitudeOfFirstGridPointInDegrees"},
        {"east", "longitudeOfLastGridPointInDegrees_fix_for_global_reduced_grids", is("gridType", "reduced_gg")},
        {"east", "longitudeOfLastGridPointInDegrees"},

        {"north", "latitudeOfFirstGridPointInDegrees", is("scanningMode", 0L)},
        {"south", "latitudeOfLastGridPointInDegrees", is("scanningMode", 0L)},

        {"north", "latitudeOfLastGridPointInDegrees", is("jScansPositively", 1L)},
        {"south", "latitudeOfFirstGridPointInDegrees", is("jScansPositively", 1L)},
        {"north", "latitudeOfFirstGridPointInDegrees"},
        {"south", "latitudeOfLastGridPointInDegrees"},

        {"truncation", "pentagonalResolutionParameterJ"},  // Assumes triangular truncation

        {"south_pole_latitude", "latitudeOfSouthernPoleInDegrees"},
        {"south_pole_longitude", "longitudeOfSouthernPoleInDegrees"},
        {"south_pole_rotation_angle", "angleOfRotationInDegrees"},

        {"proj", "projTargetString"},
        {"projSource", "projSourceString"},

        // This will be just called for has()
        {
            "gridded",
            "Nx",
            _or(_or(_or(is("gridType", "polar_stereographic"), is("gridType", "lambert_azimuthal_equal_area")),
                    is("gridType", "lambert")),
                is("gridType", "space_view")),
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
        {
            "gridded",
            "Nside" /*just a dummy*/,
            is("gridType", "healpix"),
        },
        {"gridded", "numberOfPointsAlongAMeridian"},  // Is that always true?
        {"gridded_regular_ll", "Ni", _or(is("gridType", "regular_ll"), is("gridType", "rotated_ll"))},

        {"grid", "gridName",
         _or(_or(_or(_or(is("gridType", "regular_gg"), is("gridType", "reduced_gg")), is("gridType", "rotated_gg")),
                 is("gridType", "reduced_rotated_gg")),
             is("gridType", "unstructured_grid"))},

        {"grid", "gridName_fix_for_healpix_grids", is("gridType", "healpix")},

        {"spectral", "pentagonalResolutionParameterJ"},

        {"uid", "uuidOfHGrid", is("gridType", "unstructured_grid")},

        /// FIXME: Find something that does no clash
        {"reduced", "numberOfParallelsBetweenAPoleAndTheEquator", is("isOctahedral", 0L)},
        {"regular", "N", is("gridType", "regular_gg")},
        {"octahedral", "numberOfParallelsBetweenAPoleAndTheEquator", is("isOctahedral", 1L)},

        /// TODO: is that a good idea?
        {"param", "paramId"},
        {"statistics", ""},  // (avoid ecCodes error "statistics: Function not yet implemented")
    };

    for (const auto& m : mappings) {
        if (name == m.name) {
            if (!m.condition || m.condition->eval(h)) {
                return m.key;
            }
        }
    }

    const auto* key = name.c_str();
    return key;
}


template <typename T>
struct ProcessingT {
    using fun_t = std::function<bool(grib_handle*, T&)>;
    fun_t fun_;
    ProcessingT(fun_t&& fun) : fun_(fun) {}
    ~ProcessingT()                     = default;
    ProcessingT(const ProcessingT&)    = delete;
    ProcessingT(ProcessingT&&)         = delete;
    void operator=(const ProcessingT&) = delete;
    void operator=(ProcessingT&&)      = delete;
    bool eval(grib_handle* h, T& v) const { return fun_(h, v); }
};


struct WindCache {
    static const util::Wind::Defaults& defaults() {
        static const util::Wind::Defaults def;
        return def;
    };
} static const WIND;


static ProcessingT<long>* is_wind_component_uv() {
    return new ProcessingT<long>([](grib_handle* h, long& value) {
        long paramId = 0;
        GRIB_CALL(codes_get_long(h, "paramId", &paramId));
        const auto id = paramId % 1000;
        return value  = id == WIND.defaults().u ? 1 : id == WIND.defaults().v ? 2 : 0;
    });
}


static ProcessingT<long>* is_wind_component_vod() {
    return new ProcessingT<long>([](grib_handle* h, long& value) {
        long paramId = 0;
        GRIB_CALL(codes_get_long(h, "paramId", &paramId));
        const auto id = paramId % 1000;
        return value  = id == WIND.defaults().vo ? 1 : id == WIND.defaults().d ? 2 : 0;
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
        for (const auto& key : keys_) {
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
        for (auto& prefix : std::vector<std::string>{"grid_", "spectral_"}) {
            if (packingType.find(prefix) == 0) {
                value = packingType.substr(prefix.size());
                std::replace(value.begin(), value.end(), '_', '-');
                return true;
            }
        }

        return false;
    });
}


static ProcessingT<std::string>* gridName_fix_for_healpix_grids() {
    return new ProcessingT<std::string>([](grib_handle* h, std::string& value) {
        std::string gridName;

        char buffer[64];
        size_t size = sizeof(buffer);

        GRIB_CALL(codes_get_string(h, "gridName", buffer, &size));
        ASSERT(size < sizeof(buffer) - 1);

        if (::strcmp(buffer, "MISSING") != 0) {
            gridName += buffer;
        }

        size = sizeof(buffer);
        GRIB_CALL(codes_get_string(h, "orderingConvention", buffer, &size));
        ASSERT(size < sizeof(buffer) - 1);

        if (::strcmp(buffer, "MISSING") != 0) {
            if (::strcmp(buffer, "nested") == 0) {
                gridName += "_nested";
            }
        }

        if (!gridName.empty()) {
            value = gridName;
            return true;
        }

        return false;
    });
}


template <class T>
struct ConditionedProcessingT {
    const std::string name;
    const std::unique_ptr<const T> processing;
    const std::unique_ptr<const Condition> condition;
    ConditionedProcessingT(const std::string& _name, const T* _processing, const Condition* _condition = nullptr) :
        name(_name), processing(_processing), condition(_condition) {}
};


template <class T>
using ProcessingList = std::initializer_list<ConditionedProcessingT<ProcessingT<T>>>;


template <typename T>
static bool get_value(const std::string& name, grib_handle* h, T& value, const ProcessingList<T>& process) {
    for (auto& p : process) {
        if (name == p.name) {
            if (!p.condition || p.condition->eval(h)) {
                ASSERT(p.processing);
                return p.processing->eval(h, value);
            }
        }
    }
    return false;
}


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
    util::lock_guard<util::recursive_mutex> lock(mutex_);

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
    GRIB_CALL(codes_get_double_array(grib_, "values", values.data(), &size));
    ASSERT(count == size);

    long missingValuesPresent;
    GRIB_CALL(codes_get_long(grib_, "missingValuesPresent", &missingValuesPresent));

    double missingValue;
    GRIB_CALL(codes_get_double(grib_, "missingValue", &missingValue));

    // Ensure missingValue is unique, so values are not wrongly "missing"
    long numberOfMissingValues = 0;
    GRIB_GET(codes_get_long(grib_, "numberOfMissingValues", &numberOfMissingValues));
    if (numberOfMissingValues == 0) {
        grib_get_unique_missing_value(values, missingValue);
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

        if (auto pl_sum = static_cast<size_t>(std::accumulate(pl.begin(), pl.end(), 0L)); pl_sum != values.size()) {
            wrongly_encoded_grib("GribInput: sum of pl array (" + std::to_string(pl_sum) +
                                 ") does not match the size of values array (" + std::to_string(values.size()) + ")");
        }

        // NOTE: this fix ties with the method get(const std::string &name, std::vector<long> &value)
        if (std::find(pl.rbegin(), pl.rend(), 0) != pl.rend()) {

            // if there are no missing values yet, set them
            if (missingValuesPresent == 0) {
                Log::debug() << "GribInput: introducing missing values (setting bitmap)" << std::endl;
                missingValuesPresent = 1;
                grib_get_unique_missing_value(values, missingValue);
            }

            // pl array: insert entries in place of zeros
            std::vector<long> pl_fixed = pl;
            size_t new_values          = fix_pl_array_zeros(pl_fixed);
            ASSERT(new_values > 0);

            // values array: copy values row by row, and when a fixed (0) entry is found, insert missing values
            Log::debug() << "GribInput: correcting values array with " << new_values << " new missing values"
                         << std::endl;

            MIRValuesVector values_extended;
            values_extended.reserve(count + new_values);

            ASSERT(pl.size() == pl_fixed.size());
            size_t i = 0;
            for (auto p1 = pl.begin(), p2 = pl_fixed.begin(); p1 != pl.end(); ++p1, ++p2) {
                if (*p1 == 0) {
                    ASSERT(*p2 > 0);
                    auto Ni = static_cast<size_t>(*p2);
                    values_extended.insert(values_extended.end(), Ni, missingValue);
                }
                else {
                    auto Ni = static_cast<size_t>(*p1);
                    ASSERT(i + Ni <= count);

                    values_extended.insert(values_extended.end(), &values[i], &values[i + Ni]);
                    i += Ni;
                }
            }

            // set the new (extended) values vector, confirm it is compatible with a returned pl array
            ASSERT(values.size() + new_values == values_extended.size());
            values.swap(values_extended);

            ASSERT(get("pl", pl));
            auto pl_sum = static_cast<size_t>(std::accumulate(pl.begin(), pl.end(), 0L));
            ASSERT(pl_sum == values.size());
        }
    }

    data::MIRField field(cache_, missingValuesPresent != 0, missingValue);

    long scanningMode = 0;
    GRIB_GET(codes_get_long(grib_, "scanningMode", &scanningMode));
    if (scanningMode != 0) {
        field.representation()->reorder(scanningMode, values);
    }

    field.update(values, 0);
    field.validate();

    return field;
}


grib_handle* GribInput::gribHandle(size_t which) const {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    ASSERT(which == 0);
    return grib_;
}


bool GribInput::has(const std::string& name) const {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    ASSERT(grib_);
    const auto* key = get_key(name, grib_);

    ASSERT(key != nullptr);
    if (std::strlen(key) == 0) {
        return false;
    }

    bool ok = codes_is_defined(grib_, key) != 0;

    // Log::debug() << "GribInput::has(" << name << ",key=" << key << ") " << (ok ? "yes" : "no") <<
    // std::endl;
    return ok;
}


bool GribInput::get(const std::string& name, bool& value) const {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    ASSERT(grib_);
    const auto* key = get_key(name, grib_);

    ASSERT(key != nullptr);
    if (std::strlen(key) == 0) {
        return false;
    }

    // NOTE: They key has to return a non-zero value
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
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    ASSERT(grib_);
    const std::string key = get_key(name, grib_);
    if (key.empty()) {
        return false;
    }

    // FIXME: make sure that 'value' is not set if CODES_MISSING_LONG
    int err = codes_get_long(grib_, key.c_str(), &value);
    if (err == CODES_NOT_FOUND || codes_is_missing(grib_, key.c_str(), &err) != 0) {
        static const ProcessingList<long> process{
            {"is_wind_component_uv", is_wind_component_uv()},
            {"is_wind_component_vod", is_wind_component_vod()},
        };

        return get_value(key, grib_, value, process) || FieldParametrisation::get(name, value);
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
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    ASSERT(grib_);

    ASSERT(name != "grid");

    const auto* key = get_key(name, grib_);

    ASSERT(key != nullptr);
    if (std::strlen(key) == 0) {
        return false;
    }

    // FIXME: make sure that 'value' is not set if CODES_MISSING_DOUBLE
    int err = codes_get_double(grib_, key, &value);
    if (err == CODES_NOT_FOUND || codes_is_missing(grib_, key, &err) != 0) {
        static const ProcessingList<double> process{
            {"angular_precision", angular_precision()},
            {"longitudeOfLastGridPointInDegrees_fix_for_global_reduced_grids",
             longitudeOfLastGridPointInDegrees_fix_for_global_reduced_grids()},
            {"iDirectionIncrementInDegrees_fix_for_periodic_regular_grids",
             iDirectionIncrementInDegrees_fix_for_periodic_regular_grids()},
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


bool GribInput::get(const std::string& /*name*/, std::vector<int>& /*value*/) const {
    NOTIMP;
}


bool GribInput::get(const std::string& name, std::vector<long>& value) const {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    ASSERT(grib_);
    const auto* key = get_key(name, grib_);

    ASSERT(key != nullptr);
    if (std::strlen(key) == 0) {
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

    GRIB_CALL(codes_get_long_array(grib_, key, value.data(), &size));
    ASSERT(count == size);

    ASSERT(!value.empty());

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
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    ASSERT(grib_);
    const auto* key = get_key(name, grib_);

    ASSERT(key != nullptr);
    if (std::strlen(key) == 0) {
        return false;
    }

    char buffer[10240];
    size_t size = sizeof(buffer);
    int err     = codes_get_string(grib_, key, buffer, &size);

    if (err == CODES_NOT_FOUND) {
        static const ProcessingList<std::string> process{
            {"packing", packing()}, {"gridName_fix_for_healpix_grids", gridName_fix_for_healpix_grids()}};

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
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    ASSERT(grib_);
    const auto* key = get_key(name, grib_);

    // NOTE: MARS client sets 'grid=vector' (deprecated) which needs to be compared against GRIB gridName
    ASSERT(key != nullptr);
    if (std::strlen(key) == 0 || std::strncmp(key, "gridName", 8) == 0) {
        return false;
    }

    static const ProcessingList<std::vector<double>> process{
        {"grid", vector_double({"iDirectionIncrementInDegrees", "jDirectionIncrementInDegrees"}),
         _or(is("gridType", "regular_ll"), is("gridType", "rotated_ll"))},
        {"grid", vector_double({"xDirectionGridLengthInMetres", "yDirectionGridLengthInMetres"}),
         is("gridType", "lambert_azimuthal_equal_area")},
        {"grid", vector_double({"DxInMetres", "DyInMetres"}),
         _or(is("gridType", "lambert"), is("gridType", "polar_stereographic"))},
        {"grid", vector_double({"DiInMetres", "DjInMetres"}), is("gridType", "mercator")},
        {"grid", vector_double({"dx", "dy"}), is("gridType", "space_view")},
        {"rotation", vector_double({"latitudeOfSouthernPoleInDegrees", "longitudeOfSouthernPoleInDegrees"}),
         _or(_or(_or(is("gridType", "rotated_ll"), is("gridType", "rotated_gg")), is("gridType", "rotated_sh")),
             is("gridType", "reduced_rotated_gg"))},
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

    GRIB_CALL(codes_get_double_array(grib_, key, value.data(), &size));
    ASSERT(count == size);

    // Log::debug() << "codes_get_double_array(" << name << ",key=" << key << ") size=" << value.size()
    // << std::endl;

    ASSERT(!value.empty());
    return true;
}


bool GribInput::get(const std::string& /*name*/, std::vector<std::string>& /*value*/) const {
    NOTIMP;
}


bool GribInput::handle(grib_handle* h) {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    FieldParametrisation::reset();
    cache_.reset();

    codes_handle_delete(grib_);
    grib_ = h;

    if (h != nullptr) {
        long value = 0;
        GRIB_CALL(codes_get_long(h, "7777", &value));

        // apply user-defined fixes, if any
        static const grib::Config config(LibMir::configFile(LibMir::config_file::GRIB_INPUT), false);
        config.find(parametrisation(0)).copyValuesTo(cache_.cache_);

        return true;
    }

    return false;
}


void GribInput::setAuxiliaryInformation(const util::ValueMap& map) {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    auto load = [](const eckit::PathName& path, std::vector<double>& values) {
        Log::info() << "GribInput::setAuxiliaryInformation: '" << path << "'" << std::endl;

        static const param::DefaultParametrisation param;
        std::unique_ptr<input::MIRInput> input(input::MIRInputFactory::build(path.asString(), param));
        ASSERT(input->next());

        auto field = input->field();
        ASSERT(field.dimensions() == 1);

        values = field.values(0);
    };

    for (const auto& kv : map) {
        if (kv.first == "griddef") {
            GriddefInput::load(kv.second, latitudes_, longitudes_);
            ASSERT(latitudes_.size() == longitudes_.size());
        }
        else if (kv.first == "latitudes") {
            load(kv.second, latitudes_);
        }
        else if (kv.first == "longitudes") {
            load(kv.second, longitudes_);
        }
    }
}


bool GribInput::only(size_t paramId) {
    auto paramIdOnly = long(paramId);

    while (next()) {
        util::lock_guard<util::recursive_mutex> lock(mutex_);

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
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    values.clear();
    values.reserve(latitudes_.size());
    std::copy(latitudes_.begin(), latitudes_.end(), std::back_inserter(values));
}


void GribInput::longitudes(std::vector<double>& values) const {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    values.clear();
    values.reserve(longitudes_.size());
    std::copy(longitudes_.begin(), longitudes_.end(), std::back_inserter(values));
}


void GribInput::marsRequest(std::ostream& out) const {
    util::lock_guard<util::recursive_mutex> lock(mutex_);

    ASSERT(grib_);

    static std::string gribToRequestNamespace = eckit::Resource<std::string>("gribToRequestNamespace", "mars");

    auto* keys = codes_keys_iterator_new(grib_, CODES_KEYS_ITERATOR_ALL_KEYS, gribToRequestNamespace.c_str());
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


}  // namespace mir::input
