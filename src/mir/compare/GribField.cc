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


#include "GribField.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <sstream>

#include "eckit/config/Resource.h"
#include "eckit/log/JSON.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/serialisation/MemoryStream.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/util/Grib.h"
#include "mir/util/Log.h"


namespace mir {
namespace compare {


static bool ignoreAccuracy_           = false;
static bool ignorePacking_            = false;
static bool whiteListAccuracyPacking_ = false;


static double areaPrecisionN_ = 0.;
static double areaPrecisionW_ = 0.;
static double areaPrecisionS_ = 0.;
static double areaPrecisionE_ = 0.;


void GribField::addOptions(std::vector<eckit::option::Option*>& options) {
    using eckit::option::SimpleOption;

    options.push_back(
        new SimpleOption<double>("compare-areas-threshold", "Threshold when comparing areas with Jaccard distance"));

    options.push_back(
        new SimpleOption<double>("value-count-comparison-threshold", "Threshold when comparing number of values"));

    options.push_back(new SimpleOption<bool>("ignore-accuracy", "Ignore accuracy when comparing"));

    options.push_back(new SimpleOption<bool>("ignore-packing", "Ignore packing when comparing"));

    options.push_back(new SimpleOption<double>("area-precision-north",
                                               "Epsilon when comparing latitude and longitude of bounding box"));
    options.push_back(new SimpleOption<double>("area-precision-west",
                                               "Epsilon when comparing latitude and longitude of bounding box"));
    options.push_back(new SimpleOption<double>("area-precision-south",
                                               "Epsilon when comparing latitude and longitude of bounding box"));
    options.push_back(new SimpleOption<double>("area-precision-east",
                                               "Epsilon when comparing latitude and longitude of bounding box"));

    options.push_back(
        new SimpleOption<bool>("white-list-accuracy-packing", "Report difference with accuracy & packing"));
}


void GribField::setOptions(const eckit::option::CmdArgs& args) {
    args.get("ignore-accuracy", ignoreAccuracy_);
    args.get("ignore-packing", ignorePacking_);
    args.get("area-precision-north", areaPrecisionN_);
    args.get("area-precision-west", areaPrecisionW_);
    args.get("area-precision-south", areaPrecisionS_);
    args.get("area-precision-east", areaPrecisionE_);
    args.get("white-list-accuracy-packing", whiteListAccuracyPacking_);
}


GribField::GribField(const std::string& path, off_t offset, size_t length) :
    FieldBase(path, offset, length),
    param_(-1),
    north_(0),
    west_(0),
    south_(0),
    east_(0),
    accuracy_(-1),
    decimalScaleFactor_(0),
    west_east_(0),
    north_south_(0),
    rotation_latitude_(0),
    rotation_longitude_(0),
    resol_(-1),
    numberOfPoints_(-1),
    area_(false),
    grid_(false),
    rotation_(false),
    hasMissing_(false) {}


void GribField::insert(const std::string& key, const std::string& value) {
    values_[key] = value;
}


void GribField::insert(const std::string& key, long value) {
    std::ostringstream oss;
    oss << value;
    insert(key, oss.str());
}


void GribField::erase(const std::string& key) {
    values_.erase(key);
}


const std::string& GribField::format() const {
    return format_;
}


bool GribField::canCompareFieldValues() const {
    return true;
}


// double GribField::compare(const GribField& other) const {
//     return compareExtra(other);
// }


void GribField::compareExtra(std::ostream& out, const FieldBase& o) const {
    const auto& other = dynamic_cast<const GribField&>(o);

    if (!area_ || !other.area_) {
        return;
    }

    double w1 = normaliseLongitude(west_);
    double e1 = normaliseLongitude(east_);
    double n1 = north_;
    double s1 = south_;

    double w2 = normaliseLongitude(other.west_);
    double e2 = normaliseLongitude(other.east_);
    double n2 = other.north_;
    double s2 = other.south_;

    out << std::abs(n1 - n2) << '/' << std::abs(w1 - w2) << '/' << std::abs(s1 - s2) << '/' << std::abs(e1 - e2);

    out << " [" << (std::abs(n1 - n2) - areaPrecisionN_) << '/' << (std::abs(w1 - w2) - areaPrecisionW_) << '/'
        << (std::abs(s1 - s2) - areaPrecisionS_) << '/' << (std::abs(e1 - e2) - areaPrecisionE_) << "]";
}


bool GribField::sameArea(const GribField& other) const {

    if (!area_ && !other.area_) {
        return true;
    }

    if (area_ != other.area_) {
        return false;
    }

    using eckit::types::is_approximately_equal;
    return is_approximately_equal(north_, other.north_, areaPrecisionN_) &&
           is_approximately_equal(south_, other.south_, areaPrecisionS_) &&
           is_approximately_equal(normaliseLongitude(west_), normaliseLongitude(other.west_), areaPrecisionW_) &&
           is_approximately_equal(normaliseLongitude(east_), normaliseLongitude(other.east_), areaPrecisionE_);

    // return compareExtra(other) > areaComparisonThreshold_;
}


void GribField::missingValuesPresent(bool on) {
    hasMissing_ = on;
}


void GribField::resol(long resol) {
    resol_ = resol;
}


void GribField::param(long param) {
    param_ = param;
}


void GribField::numberOfPoints(long n) {
    numberOfPoints_ = n;
}


void GribField::gridname(const std::string& name) {
    gridname_ = name;
}


void GribField::format(const std::string& format) {
    format_ = format;
}


void GribField::gridtype(const std::string& type) {
    gridtype_ = type;
}


bool GribField::samePacking(const GribField& other) const {

    if (accuracy_ == 0 || other.accuracy_ == 0) {
        return true;
    }

    if (packing_ == "grid_simple_matrix" && other.packing_ == "grid_simple") {
        return true;
    }

    if (packing_ == "grid_simple" && other.packing_ == "grid_simple_matrix") {
        return true;
    }

    if (ignorePacking_) {

        if (packing_ == "grid_second_order" && other.packing_ == "grid_simple") {
            return true;
        }

        if (packing_ == "grid_simple" && other.packing_ == "grid_second_order") {
            return true;
        }

        if (packing_ == "grid_jpeg" && other.packing_ == "grid_simple") {
            return true;
        }

        if (packing_ == "grid_simple" && other.packing_ == "grid_jpeg") {
            return true;
        }
    }

    return packing_ == other.packing_;
}


bool GribField::sameResol(const GribField& other) const {
    return resol_ == other.resol_;
}


bool GribField::sameGridname(const GribField& other) const {
    return gridname_ == other.gridname_;
}


bool GribField::sameGridtype(const GribField& other) const {
    return gridtype_ == other.gridtype_;
}


bool GribField::sameFormat(const GribField& other) const {
    return format_ == other.format_;
}


bool GribField::sameParam(const GribField& other) const {
    return param_ == other.param_;
}


bool GribField::sameAccuracy(const GribField& other) const {

    if (ignoreAccuracy_) {
        return true;
    }

    if (accuracy_ == 0 || other.accuracy_ == 0) {
        return true;
    }

    if ((decimalScaleFactor_ != 0) || (other.decimalScaleFactor_ != 0)) {
        return decimalScaleFactor_ == other.decimalScaleFactor_;
    }

    return accuracy_ == other.accuracy_;
}


bool GribField::sameNumberOfPoints(const GribField& other) const {
    return numberOfPoints_ == other.numberOfPoints_;
}


bool GribField::sameBitmap(const GribField& /*other*/) const {
    return true;
}


bool GribField::sameGrid(const GribField& other) const {

    if (grid_ != other.grid_) {
        return false;
    }

    if (grid_) {

        return (north_south_ == other.north_south_) && (west_east_ == other.west_east_);
    }

    return true;
}


bool GribField::sameRotation(const GribField& other) const {

    if (rotation_ != other.rotation_) {
        return false;
    }

    if (rotation_) {
        return (rotation_latitude_ == other.rotation_latitude_) &&
               (normaliseLongitude(rotation_longitude_) == normaliseLongitude(other.rotation_longitude_));
    }

    return true;
}


bool GribField::less_than(const FieldBase& o) const {
    const auto& other = dynamic_cast<const GribField&>(o);

    if (param_ < other.param_) {
        return true;
    }

    if (param_ > other.param_) {
        return false;
    }

    if (format_ < other.format_) {
        return true;
    }

    if (format_ > other.format_) {
        return false;
    }

    std::string this_packing  = packing_;
    std::string other_packing = other.packing_;

    // If the field is contant, the packing is set to 'grid_simple'
    if (accuracy_ == 0) {
        this_packing = other.packing_;
    }

    if (other.accuracy_ == 0) {
        other_packing = packing_;
    }

    if (accuracy_ == 0 && other.accuracy_ == 0) {
        this_packing = other_packing = packing_;
    }

    if (this_packing < other_packing) {
        return true;
    }

    if (this_packing > other_packing) {
        return false;
    }

    if (gridtype_ < other.gridtype_) {
        return true;
    }

    if (gridtype_ > other.gridtype_) {
        return false;
    }

    if (gridname_ < other.gridname_) {
        return true;
    }

    if (gridname_ > other.gridname_) {
        return false;
    }

    if (resol_ < other.resol_) {
        return true;
    }

    if (resol_ > other.resol_) {
        return false;
    }

    // if(sameAccuracy(other)) {
    //     return false;
    // }

    long this_accuracy  = (accuracy_ != 0) ? accuracy_ : other.accuracy_;
    long other_accuracy = (other.accuracy_ != 0) ? other.accuracy_ : accuracy_;


    if (this_accuracy < other_accuracy) {
        return true;
    }

    if (this_accuracy > other_accuracy) {
        return false;
    }

    if (hasMissing_ != other.hasMissing_) {
        return other.hasMissing_;
    }

    if (numberOfPoints_ < other.numberOfPoints_) {
        return true;
    }

    if (numberOfPoints_ > other.numberOfPoints_) {
        return false;
    }

    if (grid_ != other.grid_) {
        return other.grid_;
    }

    if (grid_) {

        if (north_south_ < other.north_south_) {
            return true;
        }

        if (north_south_ > other.north_south_) {
            return false;
        }

        if (west_east_ < other.west_east_) {
            return true;
        }

        if (west_east_ > other.west_east_) {
            return false;
        }
    }

    if (area_ != other.area_) {
        return other.area_;
    }

    if (area_) {

        if (north_ < other.north_) {
            return true;
        }

        if (north_ > other.north_) {
            return false;
        }

        if (normaliseLongitude(west_) < normaliseLongitude(other.west_)) {
            return true;
        }

        if (normaliseLongitude(west_) > normaliseLongitude(other.west_)) {
            return false;
        }

        if (south_ < other.south_) {
            return true;
        }

        if (south_ > other.south_) {
            return false;
        }

        if (normaliseLongitude(east_) < normaliseLongitude(other.east_)) {
            return true;
        }

        if (normaliseLongitude(east_) > normaliseLongitude(other.east_)) {
            return false;
        }
    }

    if (rotation_ != other.rotation_) {
        return other.rotation_;
    }

    if (rotation_) {

        if (rotation_latitude_ < other.rotation_latitude_) {
            return true;
        }

        if (rotation_latitude_ > other.rotation_latitude_) {
            return false;
        }

        if (normaliseLongitude(rotation_longitude_) < normaliseLongitude(other.rotation_longitude_)) {
            return true;
        }

        if (normaliseLongitude(rotation_longitude_) > normaliseLongitude(other.rotation_longitude_)) {
            return false;
        }
    }

    return values_ < other.values_;
}


std::map<std::string, std::string>::const_iterator GribField::begin() const {
    return values_.begin();
}


std::map<std::string, std::string>::const_iterator GribField::end() const {
    return values_.end();
}


std::map<std::string, std::string>::const_iterator GribField::find(const std::string& key) const {
    return values_.find(key);
}


void GribField::area(double n, double w, double s, double e) {
    area_  = true;
    north_ = n;
    west_  = w;
    south_ = s;
    east_  = e;

    if (north_ < south_) {
        std::ostringstream oss;
        oss << "Invalid area: " << *this;
        throw exception::SeriousBug(oss.str());
    }
}


void GribField::accuracy(long n) {
    accuracy_ = n;
}


void GribField::decimalScaleFactor(long n) {
    decimalScaleFactor_ = n;
}


void GribField::packing(const std::string& packing) {
    packing_ = packing;
}


void GribField::grid(double ns, double we) {
    grid_        = true;
    north_south_ = ns;
    west_east_   = we;
}


void GribField::rotation(double lat, double lon) {
    rotation_           = true;
    rotation_latitude_  = lat;
    rotation_longitude_ = lon;
}


void GribField::print(std::ostream& out) const {

    out << std::setprecision(12);

    out << "[param=" << param_;

    if (numberOfPoints_ >= 0) {
        out << ",values=" << numberOfPoints_;
    }

    out << ",format=" << format_;

    if (!packing_.empty()) {
        out << ",packing=" << packing_;
    }

    if (!gridtype_.empty()) {
        out << ",gridtype=" << gridtype_;
    }

    if (!gridname_.empty()) {
        out << ",gridname=" << gridname_;
    }

    if (resol_ >= 0) {
        out << ",resol=" << resol_;
    }

    if (accuracy_ >= 0) {
        out << ",accuracy=" << accuracy_;
    }

    if (decimalScaleFactor_ != 0) {
        out << ",decimal_scale_factor=" << decimalScaleFactor_;
    }

    if (hasMissing_) {
        out << ",bitmap=yes";
    }

    if (grid_) {
        out << ",grid=" << west_east_ << "/" << north_south_;
    }

    if (area_) {
        out << ",area=" << north_ << "/" << west_ << "/" << south_ << "/" << east_;
    }

    if (rotation_) {
        out << ",rotation=" << rotation_latitude_ << "/" << rotation_longitude_;
    }

    for (const auto& j : values_) {
        out << "," << j.first << "=" << j.second;
    }
    // out << " - " << info_;
    out << "]";
}


void GribField::json(eckit::JSON& json) const {
    json.startObject();
    FieldBase::json(json);
    json.endObject();
}


std::ostream& GribField::printGrid(std::ostream& out) const {

    out << std::setprecision(12);

    bool comma = false;

    if (!gridname_.empty()) {
        if (comma) {
            out << ',';
        }
        comma = true;
        out << "gridname=" << gridname_;
    }

    if (resol_ >= 0) {
        if (comma) {
            out << ',';
        }
        comma = true;
        out << "resol=" << resol_;
    }

    if (grid_) {
        if (comma) {
            out << ',';
        }
        comma = true;
        out << "grid=" << west_east_ << "/" << north_south_;
    }

    if (area_) {
        if (comma) {
            out << ',';
        }
        // comma = true;
        out << "area=" << north_ << "/" << west_ << "/" << south_ << "/" << east_;
    }

    return out;
}


bool GribField::sameField(const GribField& other) const {
    return values_ == other.values_;
}


bool GribField::match(const FieldBase& o) const {
    const auto& other = dynamic_cast<const GribField&>(o);

    return sameParam(other) && sameField(other);
    // &&
    // (sameGrid(other)  || sameResol(other) || sameGridname(other))   ;
    // sameRotation(other) &&
    // sameGridname(other) &&
    // sameGridtype(other) &&
    // sameResol(other) &&
    // (compareExtra(other) > 0.1);
}


bool GribField::same(const FieldBase& o) const {
    return differences(o) == 0;
}


size_t GribField::differences(const FieldBase& o) const {
    const auto& other = dynamic_cast<const GribField&>(o);

    size_t result = (sameParam(other) ? 0 : 100) + (sameField(other) ? 0 : 1) + (sameNumberOfPoints(other) ? 0 : 1) +
                    (sameGrid(other) ? 0 : 1) + (sameAccuracy(other) ? 0 : 1) + (samePacking(other) ? 0 : 1) +
                    (sameRotation(other) ? 0 : 1) + (sameResol(other) ? 0 : 1) + (sameGridname(other) ? 0 : 1) +
                    (sameGridtype(other) ? 0 : 1) + (sameFormat(other) ? 0 : 1) + (sameArea(other) ? 0 : 1);

    return result;
}


template <class T>
static void pdiff(std::ostream& out, const T& v1, const T& v2) {
    if (v1 != v2) {
        // out << eckit::Colour::red << eckit::Colour::bold << v1 << eckit::Colour::reset;
        out << "**" << v1 << "**";
    }
    else {
        out << v1;
    }
}


void GribField::whiteListEntries(std::ostream& out) const {

    out << std::setprecision(12);

    const char* sep = "";

    if (whiteListAccuracyPacking_) {
        if (param_ != 0) {
            out << sep << "param=" << param_;
            sep = ",";
        }
        if (!format_.empty()) {
            out << sep << "format=" << format_;
            sep = ",";
        }
        if (!packing_.empty()) {
            out << sep << "packing=" << packing_;
            sep = ",";
        }
        if (!gridtype_.empty()) {
            out << sep << "gridtype=" << gridtype_;
            sep = ",";
        }
        if (accuracy_ >= 0) {
            out << sep << "accuracy=" << accuracy_;
            sep = ",";
        }
        if (decimalScaleFactor_ != 0) {
            out << sep << "decimal_scale_factor=" << decimalScaleFactor_;
            // sep = ",";
        }
    }
    else {
        if (!gridname_.empty()) {
            out << sep << "gridname=" << gridname_;
            sep = ",";
        }
        if (grid_) {
            out << sep << "grid=" << north_south_ << "/" << west_east_;
            sep = ",";
        }
        if (area_) {
            out << sep << "area=" << north_ << "/" << west_ << "/" << south_ << "/" << east_;
            sep = ",";
        }
        if (rotation_) {
            out << sep << "rotation=" << rotation_latitude_ << "/" << rotation_longitude_;
            // sep = ",";
        }
    }
}


std::ostream& GribField::printDifference(std::ostream& out, const FieldBase& o) const {

    out << std::setprecision(12);

    const auto& other = dynamic_cast<const GribField&>(o);

    out << "[param=";
    pdiff(out, param_, other.param_);

    if (numberOfPoints_ >= 0) {
        out << ",values=";
        pdiff(out, numberOfPoints_, other.numberOfPoints_);
    }

    out << ",format=";
    pdiff(out, format_, other.format_);

    if (!packing_.empty()) {
        out << ",packing=";
        pdiff(out, packing_, other.packing_);
    }

    if (!gridtype_.empty()) {
        out << ",gridtype=";
        pdiff(out, gridtype_, other.gridtype_);
    }

    if (!gridname_.empty()) {
        out << ",gridname=";
        pdiff(out, gridname_, other.gridname_);
    }

    if (resol_ >= 0) {
        out << ",resol=";
        pdiff(out, resol_, other.resol_);
    }

    if (decimalScaleFactor_ >= 0) {
        out << ",decimal_scale_factor=";
        pdiff(out, decimalScaleFactor_, other.decimalScaleFactor_);
    }
    else {
        if (accuracy_ >= 0) {
            out << ",accuracy=";
            pdiff(out, accuracy_, other.accuracy_);
        }
    }

    if (hasMissing_) {
        out << ",bitmap=yes";
    }

    if (grid_) {
        out << ",grid=";
        pdiff(out, north_south_, other.north_south_);
        out << "/";
        pdiff(out, west_east_, other.west_east_);
    }

    if (area_) {
        out << ",area=";
        pdiff(out, north_, other.north_);
        out << "/";
        pdiff(out, west_, other.west_);
        out << "/";
        pdiff(out, south_, other.south_);
        out << "/";
        pdiff(out, east_, other.east_);
    }

    if (rotation_) {
        out << ",rotation=";
        pdiff(out, rotation_latitude_, other.rotation_latitude_);
        out << "/";
        pdiff(out, rotation_longitude_, other.rotation_longitude_);
    }

    for (const auto& j : values_) {
        out << "," << j.first << "=";
        auto k = other.values_.find(j.first);
        pdiff(out, j.second, k == other.values_.end() ? std::string() : k->second);
    }

    out << ",wrapped=" << wrapped();
    // out << " - " << info_;
    out << "]";

    return out;
}


bool GribField::wrapped() const {
    if (!area_) {
        return false;
    }

    double w = normaliseLongitude(west_);
    double e = normaliseLongitude(east_);

    return w == e;
}


size_t GribField::numberOfPoints() const {
    return size_t(numberOfPoints_);
}


bool GribField::match(const std::string& name, const std::string& value) const {
    auto j = values_.find(name);
    if (j != values_.end()) {
        return j->second == value;
    }

    if (name == "area") {
        std::ostringstream oss;
        oss << std::setprecision(12);
        if (area_) {
            oss << north_ << '/' << west_ << '/' << south_ << '/' << east_;
        }
        return value == oss.str();
    }

    if (name == "grid") {
        std::ostringstream oss;
        if (grid_) {
            oss << west_east_ << '/' << north_south_;
        }
        return value == oss.str();
    }


    if (name == "param") {
        std::ostringstream oss;
        oss << param_;
        return value == oss.str();
    }


    if (name == "values") {
        std::ostringstream oss;
        oss << numberOfPoints_;
        return value == oss.str();
    }

    if (name == "format") {
        return value == format_;
    }

    if (name == "packing") {
        return value == packing_;
    }

    if (name == "gridtype") {
        return value == gridtype_;
    }

    if (name == "gridname") {
        return value == gridname_;
    }

    if (name == "resol") {
        std::ostringstream oss;
        oss << resol_;
        return value == oss.str();
    }

    if (name == "accuracy") {
        std::ostringstream oss;
        oss << accuracy_;
        return value == oss.str();
    }

    if (name == "decimal_scale_factor") {
        std::ostringstream oss;
        oss << decimalScaleFactor_;
        return value == oss.str();
    }

    if (name == "bitmap") {
        std::ostringstream oss;
        oss << (hasMissing_ ? "yes" : "no");
        return value == oss.str();
    }

    if (name == "rotation") {
        std::ostringstream oss;
        if (rotation_) {
            oss << rotation_latitude_ << '/' << rotation_longitude_;
        }
        return value == oss.str();
    }

    return false;
}


Field GribField::field(const char* buffer, size_t size, const std::string& path, off_t offset,
                       const std::vector<std::string>& ignore) {

    auto* field = new GribField(path, offset, size);
    Field result(field);

    auto* h = codes_handle_new_from_message(nullptr, buffer, size);
    HandleDeleter delh(h);

    static std::string gribToRequestNamespace = eckit::Resource<std::string>("gribToRequestNamespace", "mars");

    auto* ks = codes_keys_iterator_new(h, CODES_KEYS_ITERATOR_ALL_KEYS, gribToRequestNamespace.c_str());
    ASSERT(ks);
    GKeyIteratorDeleter delk(ks);

    /// @todo this code should be factored out into mir

    // bool sfc = false;

    std::map<std::string, std::string> req;

    while (codes_keys_iterator_next(ks) != 0) {
        const auto* name = codes_keys_iterator_get_name(ks);
        ASSERT(name);

        if ((name[0] == '_') || (::strcmp(name, "param") == 0)) {
            continue;
        }

        char val[1024];
        size_t len = sizeof(val);

        GRIB_CALL(codes_keys_iterator_get_string(ks, val, &len));

        field->insert(name, val);

        // if (::strcmp(val, "sfc") == 0) {
        //     sfc = true;
        // }

        req[name] = val;
    }


    long paramId;
    GRIB_CALL(codes_get_long(h, "paramId", &paramId));

    field->param(paramId);

    long numberOfDataPoints;
    GRIB_CALL(codes_get_long(h, "numberOfDataPoints", &numberOfDataPoints));
    field->numberOfPoints(numberOfDataPoints);

    // Look for request embbeded in GRIB message
    long local;

    if (codes_get_long(h, "localDefinitionNumber", &local) == 0 && local == 191) {
        size_t dataSize;
        /* TODO: Not grib2 compatible, but speed-up process */
        if (codes_get_size(h, "freeFormData", &dataSize) == 0 && dataSize != 0) {
            std::vector<unsigned char> data(dataSize);
            GRIB_CALL(codes_get_bytes(h, "freeFormData", data.data(), &dataSize));

            eckit::MemoryStream s(data.data(), dataSize);

            int count;
            s >> count;  // Number of requests
            ASSERT(count == 1);
            std::string tmp;
            s >> tmp;  // verb
            s >> count;
            for (int i = 0; i < count; i++) {
                std::string key;
                std::string value;
                int n;
                s >> key;
                std::transform(key.begin(), key.end(), key.begin(), tolower);
                s >> n;  // Number of values
                ASSERT(n == 1);
                s >> value;
                std::transform(value.begin(), value.end(), value.begin(), tolower);
                field->insert(key, value);
                req[key] = value;
            }
        }
    }

    {
        char value[1024];
        size_t len = sizeof(value);

        if (codes_get_string(h, "gridType", value, &len) == 0) {
            std::string v(value);
            field->gridtype(v);


            if (v == "regular_ll") {
                setGrid(*field, h);
                setArea(*field, h);
            }
            else if (v == "rotated_ll") {
                setGrid(*field, h);
                setArea(*field, h);

                double lat;
                double lon;
                GRIB_CALL(codes_get_double(h, "latitudeOfSouthernPoleInDegrees", &lat));
                GRIB_CALL(codes_get_double(h, "longitudeOfSouthernPoleInDegrees", &lon));
                field->rotation(lat, lon);
            }
            else if (v == "sh") {

                long n = -1;
                GRIB_CALL(codes_get_long(h, "pentagonalResolutionParameterJ", &n));
                field->resol(n);
            }
            else if (v == "reduced_gg") {

                long n = 0;
                std::ostringstream oss;

                GRIB_CALL(codes_get_long(h, "isOctahedral", &n));

                if (n == 0) {

                    // Don't trust eccodes
                    size_t pl_size = 0;
                    GRIB_CALL(codes_get_size(h, "pl", &pl_size));

                    std::vector<long> pl(pl_size);
                    GRIB_CALL(codes_get_long_array(h, "pl", pl.data(), &pl_size));

                    bool isOctahedral = true;
                    for (size_t i = 1; i < pl_size; i++) {
                        long diff = std::abs(pl[i] - pl[i - 1]);
                        if (diff != 4 && diff != 0) {
                            isOctahedral = false;
                            break;
                        }
                    }

                    oss << (isOctahedral ? "O" : "N");
                }
                else {
                    oss << "O";
                }

                GRIB_CALL(codes_get_long(h, "N", &n));
                oss << n;


                // ASSERT(codes_get_double(h, "iDirectionIncrementInDegrees", &d) == 0);
                // oss << '/' << rounded(d);
                field->gridname(oss.str());


                setArea(*field, h);
            }
            else if (v == "reduced_ll") {

                // Don't trust eccodes
                size_t pl_size = 0;
                GRIB_CALL(codes_get_size(h, "pl", &pl_size));

                std::vector<long> pl(pl_size);
                GRIB_CALL(codes_get_long_array(h, "pl", pl.data(), &pl_size));

                std::ostringstream oss;
                oss << "RLL" << pl.size() << "-";
                eckit::MD5 md5;
                for (auto& j : pl) {
                    md5 << j;
                }

                oss << md5.digest();
                field->gridname(oss.str());

                setArea(*field, h);
            }
            else if (v == "regular_gg") {
                long n;
                std::ostringstream oss;

                GRIB_CALL(codes_get_long(h, "N", &n));
                oss << "F" << n;

                // ASSERT(codes_get_double(h, "iDirectionIncrementInDegrees", &d) == 0);
                // oss << '/' << rounded(d);
                field->gridname(oss.str());

                setArea(*field, h);
            }
            else if (v == "polar_stereographic") {
                Log::warning() << "Ignoring polar_stereographic in " << path << std::endl;
                return result;
            }
            else {
                std::ostringstream oss;
                oss << path << ": Unknown grid [" << v << "]";
                throw exception::SeriousBug(oss.str());
            }
        }
    }

    static eckit::Translator<long, std::string> l2s;

    // long scanningMode = 0;
    // if (codes_get_long(h, "scanningMode", &scanningMode) == 0) {
    //     field->insert("scanningMode", scanningMode);
    // }

    // long decimalScaleFactor = 0;
    // if (codes_get_long(h, "decimalScaleFactor", &decimalScaleFactor) == 0) {
    //     field->insert("decimalScaleFactor", decimalScaleFactor);
    // }

    long edition;
    if (codes_get_long(h, "edition", &edition) == 0) {
        field->format("grib" + l2s(edition));
    }

    long missingValuesPresent;
    if (codes_get_long(h, "missingValuesPresent", &missingValuesPresent) == 0) {
        if (missingValuesPresent != 0) {
            field->missingValuesPresent(true);
        }
    }

    long accuracy;
    if (codes_get_long(h, "accuracy", &accuracy) == 0) {
        field->accuracy(accuracy);
    }

    long decimalScaleFactor;
    if (codes_get_long(h, "decimalScaleFactor", &decimalScaleFactor) == 0) {
        field->decimalScaleFactor(decimalScaleFactor);
    }

    {
        char value[1024];
        size_t len = sizeof(value);
        if (codes_get_string(h, "packingType", value, &len) == 0) {
            field->packing(value);
        }
    }

    {
        char value[1024];
        size_t len = sizeof(value);
        if (codes_get_string(h, "packing", value, &len) == 0) {
            field->packing(value);
        }
    }

    for (const auto& j : ignore) {
        field->erase(j);
    }

    return result;
}


void GribField::setArea(GribField& field, grib_handle* h) {
    double n = -99999;
    double w = -99999;
    double s = -99999;
    double e = -99999;
    GRIB_CALL(codes_get_double(h, "latitudeOfFirstGridPointInDegrees", &n));
    GRIB_CALL(codes_get_double(h, "longitudeOfFirstGridPointInDegrees", &w));
    GRIB_CALL(codes_get_double(h, "latitudeOfLastGridPointInDegrees", &s));
    GRIB_CALL(codes_get_double(h, "longitudeOfLastGridPointInDegrees", &e));

    long scanningMode = 0;
    GRIB_CALL(codes_get_long(h, "scanningMode", &scanningMode));

    switch (scanningMode) {
        case 0:
            break;

        case 64:
            std::swap(n, s);
            break;

        default: {
            std::ostringstream oss;
            oss << "Invalid scanning mode " << scanningMode;
            throw exception::SeriousBug(oss.str());
        } /*break;*/
    }

    field.area(n, w, s, e);
}


void GribField::setGrid(GribField& field, grib_handle* h) {
    double we = -99999;
    double ns = -99999;
    GRIB_CALL(codes_get_double(h, "jDirectionIncrementInDegrees", &ns));
    GRIB_CALL(codes_get_double(h, "iDirectionIncrementInDegrees", &we));
    field.grid(ns, we);
}


}  // namespace compare
}  // namespace mir
