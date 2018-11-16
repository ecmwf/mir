/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include "mir/compare/GribField.h"

#include <cmath>

#include "eckit/config/Resource.h"
#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"
#include "eckit/serialisation/MemoryStream.h"

#include "mir/util/Grib.h"
#include "eckit/parser/JSON.h"


namespace mir {
namespace compare {


static bool ignoreAccuracy_ = false;
static bool ignorePacking_ = false;
static bool whiteListAccuracyPacking_ = false;


static double areaPrecisionN_ = 0.;
static double areaPrecisionW_ = 0.;
static double areaPrecisionS_ = 0.;
static double areaPrecisionE_ = 0.;


void GribField::addOptions(std::vector<eckit::option::Option*>& options) {
    using namespace eckit::option;

    options.push_back(new SimpleOption<double>("compare-areas-threshold",
                      "Threshold when comparing areas with Jaccard distance"));

    options.push_back(new SimpleOption<double>("value-count-comparison-threshold",
                      "Threshold when comparing number of values"));

    options.push_back(new SimpleOption<bool>("ignore-accuracy",
                      "Ignore accuracy when comparing"));

    options.push_back(new SimpleOption<bool>("ignore-packing",
                      "Ignore packing when comparing"));

    options.push_back(new SimpleOption<double>("area-precision-north",
                      "Epsilon when comparing latitude and logitude of bounding box"));
    options.push_back(new SimpleOption<double>("area-precision-west",
                      "Epsilon when comparing latitude and logitude of bounding box"));
    options.push_back(new SimpleOption<double>("area-precision-south",
                      "Epsilon when comparing latitude and logitude of bounding box"));
    options.push_back(new SimpleOption<double>("area-precision-east",
                      "Epsilon when comparing latitude and logitude of bounding box"));

    options.push_back(new SimpleOption<bool>("white-list-accuracy-packing",
                      "Report difference with accuracy & packing"));

}




void GribField::setOptions(const eckit::option::CmdArgs &args) {
    args.get("ignore-accuracy", ignoreAccuracy_);
    args.get("ignore-packing", ignorePacking_);
    args.get("area-precision-north", areaPrecisionN_);
    args.get("area-precision-west", areaPrecisionW_);
    args.get("area-precision-south", areaPrecisionS_);
    args.get("area-precision-east", areaPrecisionE_);
    args.get("white-list-accuracy-packing", whiteListAccuracyPacking_);
}


GribField::GribField(const std::string& path, off_t offset, size_t length):
    FieldBase(path, offset, length),
    param_(-1),
    area_(false),
    north_(0),
    west_(0),
    south_(0),
    east_(0),
    accuracy_(-1),
    decimalScaleFactor_(0),
    grid_(false),
    west_east_(0),
    north_south_(0),
    rotation_(false),
    rotation_latitude_(0),
    rotation_longitude_(0),
    hasMissing_(false),
    resol_(-1),
    numberOfPoints_(-1) {}

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

// double GribField::compare(const GribField& other) const {
//     return compareExtra(other);
// }


void GribField::compareExtra(std::ostream& out, const FieldBase& o) const {
    const GribField& other = dynamic_cast<const GribField&>(o);

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

    out << ::fabs(n1 - n2) << '/' << ::fabs(w1 - w2) << '/' << ::fabs(s1 - s2) << '/' << ::fabs(e1 - e2);

    out << " [" << (::fabs(n1 - n2) - areaPrecisionN_ ) << '/' << (::fabs(w1 - w2) - areaPrecisionW_) << '/' << (::fabs(s1 - s2) - areaPrecisionS_) << '/' << (::fabs(e1 - e2) - areaPrecisionE_) << "]";

}


inline bool sameLatLon(double a, double b, double e) {
    return ::fabs(a - b) <= e;
}


bool GribField::sameArea(const GribField& other) const {

    if (!area_ && !other.area_)
        return true;

    if (area_ != other.area_)
        return false;

    double w1 = normaliseLongitude(west_);
    double e1 = normaliseLongitude(east_);
    double n1 = north_;
    double s1 = south_;

    double w2 = normaliseLongitude(other.west_);
    double e2 = normaliseLongitude(other.east_);
    double n2 = other.north_;
    double s2 = other.south_;

    if (!sameLatLon(n1, n2, areaPrecisionN_)) {
        return false;
    }

    if (!sameLatLon(w1, w2, areaPrecisionW_)) {
        return false;
    }

    if (!sameLatLon(s1, s2, areaPrecisionS_)) {
        return false;
    }

    if (!sameLatLon(e1, e2, areaPrecisionE_)) {
        return false;
    }

    return true;

    // return compareExtra(other) > areaComparisonThreshold_;
}


void GribField::missingValuesPresent(bool on)  {
    hasMissing_ = on;
}

void GribField::resol(size_t resol)  {
    resol_ = resol;
}

void GribField::param(long param)  {
    param_ = param;
}

void GribField::numberOfPoints(long n)  {
    numberOfPoints_ = n;
}

void GribField::gridname(const std::string& name)  {
    gridname_ = name;
}

void GribField::format(const std::string& format)  {
    format_ = format;
}

void GribField::gridtype(const std::string& type)  {
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

    if (decimalScaleFactor_ ||  other.decimalScaleFactor_) {
        return decimalScaleFactor_  == other.decimalScaleFactor_;
    }

    return accuracy_  == other.accuracy_;
}

bool GribField::sameNumberOfPoints(const GribField& other) const {
    return numberOfPoints_ == other.numberOfPoints_;
}

bool GribField::sameBitmap(const GribField& other) const {
    return true;
}

bool GribField::sameGrid(const GribField& other) const {

    if (grid_ != other.grid_) {
        return false;
    }

    if (grid_) {

        return (north_south_ == other.north_south_) &&
               (west_east_ == other.west_east_) ;
    }

    return true;
}


bool GribField::sameRotation(const GribField& other) const {

    if (rotation_ != other.rotation_) {
        return false;
    }

    if (rotation_) {

        return (rotation_latitude_ == other.rotation_latitude_) &&
               (normaliseLongitude(rotation_longitude_) == normaliseLongitude(other.rotation_longitude_)) ;
    }

    return true;
}

bool GribField::less_than(const FieldBase & o) const {
    const GribField & other = dynamic_cast<const GribField &>(o);

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


    std::string this_packing = packing_;
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

    long this_accuracy = accuracy_ ? accuracy_ : other.accuracy_;
    long other_accuracy = other.accuracy_ ? other.accuracy_ : accuracy_;


    if (this_accuracy < other_accuracy) {
        return true;
    }

    if (this_accuracy > other_accuracy) {
        return false;
    }

    if (hasMissing_ < other.hasMissing_) {
        return true;
    }

    if (hasMissing_ > other.hasMissing_) {
        return false;
    }

    if (numberOfPoints_ < other.numberOfPoints_) {
        return true;
    }

    if (numberOfPoints_ > other.numberOfPoints_) {
        return false;
    }


    if (grid_ < other.grid_) {
        return true;
    }


    if (grid_ > other.grid_) {
        return false;
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

    if (area_ < other.area_) {
        return true;
    }

    if (area_ > other.area_) {
        return false;
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

    if (rotation_ < other.rotation_) {
        return true;
    }

    if (rotation_ > other.rotation_) {
        return false;
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

std::map<std::string, std::string>::const_iterator GribField::find(const std::string & key) const {
    return values_.find(key);
}


void GribField::area(double n, double w, double s, double e) {
    area_ = true;
    north_ = n;
    west_ = w;
    south_ = s;
    east_ = e;

    if (north_ < south_) {
        std::ostringstream oss;
        oss << "Invalid area: " << *this;
        throw eckit::SeriousBug(oss.str());
    }
}

void GribField::accuracy(long n) {
    accuracy_ = n;
}

void GribField::decimalScaleFactor(long n) {
    decimalScaleFactor_ = n;
}

void GribField::packing(const std::string & packing) {
    packing_ = packing;
}


void GribField::grid(double ns, double we) {
    grid_ = true;
    north_south_ = ns;
    west_east_ = we;
}

void GribField::rotation(double lat, double lon) {
    rotation_ = true;
    rotation_latitude_ = lat;
    rotation_longitude_ = lon;
}

void GribField::print(std::ostream & out) const {

    out << std::setprecision(12);

    out << "[param=" << param_ ;

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

    if (decimalScaleFactor_) {
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

    for (auto j = values_.begin(); j != values_.end(); ++j) {
        out << "," << (*j).first << "=" << (*j).second;
    }
    // out << " - " << info_;
    out << "]";
}

void GribField::json(eckit::JSON& json) const {
    json.startObject();
    FieldBase::json(json);
    json.endObject();
}


std::ostream & GribField::printGrid(std::ostream & out) const {

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
        comma = true;
        out << "area=" << north_ << "/" << west_ << "/" << south_ << "/" << east_;
    }

    return out;

}

bool GribField::sameField(const GribField & other) const {
    return values_ == other.values_;
}


bool GribField::match(const FieldBase & o) const {
    const GribField& other = dynamic_cast<const GribField&>(o);

    return sameParam(other) && sameField(other);
    // &&
    // (sameGrid(other)  || sameResol(other) || sameGridname(other))   ;
    // sameRotation(other) &&
    // sameGridname(other) &&
    // sameGridtype(other) &&
    // sameResol(other) &&
    // (compareExtra(other) > 0.1);
}


bool GribField::same(const FieldBase & o) const {
    const GribField& other = dynamic_cast<const GribField&>(o);

    return sameParam(other) &&
           sameField(other) &&
           sameNumberOfPoints(other) &&
           sameGrid(other) &&
           sameAccuracy(other) &&
           samePacking(other) &&
           sameRotation(other) &&
           sameResol(other) &&
           sameGridname(other) &&
           sameGridtype(other) &&
           sameFormat(other) &&
           sameArea(other);
}

size_t GribField::differences(const FieldBase & o) const {

    const GribField& other = dynamic_cast<const GribField&>(o);

    size_t result = 0;
    if (!sameParam(other)) result += 100;
    if (!sameField(other)) result++;
    if (!sameNumberOfPoints(other)) result++;
    if (!sameGrid(other)) result++;
    if (!sameAccuracy(other)) result++;
    if (!samePacking(other)) result++;
    if (!sameRotation(other)) result++;
    if (!sameResol(other)) result++;
    if (!sameGridname(other)) result++;
    if (!sameGridtype(other)) result++;
    if (!sameFormat(other)) result++;
    if (!sameArea(other)) result++;
    return result;
}


template<class T>
static void pdiff(std::ostream & out, const T& v1, const T& v2) {
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
        if (param_) {
            out << sep << "param=" << param_; sep = ",";
        }
        if (format_.length()) {
            out << sep << "format=" << format_; sep = ",";
        }
        if (packing_.length()) {
            out << sep << "packing=" << packing_; sep = ",";
        }
        if (gridtype_.length()) {
            out << sep << "gridtype=" << gridtype_; sep = ",";
        }
        if (accuracy_ >= 0) {
            out << sep << "accuracy=" << accuracy_; sep = ",";
        }
        if (decimalScaleFactor_) {
            out << sep << "decimal_scale_factor=" << decimalScaleFactor_; sep = ",";
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
            out << sep << "area=" << north_ << "/" << west_ << "/" <<  south_ << "/" << east_;
            sep = ",";
        }

        if (rotation_) {
            out << sep << "rotation=" << rotation_latitude_ << "/"  << rotation_longitude_;
            sep = ",";
        }
    }
}


std::ostream& GribField::printDifference(std::ostream & out, const FieldBase & o) const {

    out << std::setprecision(12);


    const GribField& other = dynamic_cast<const GribField&>(o);

    out << "[param=";
    pdiff(out, param_, other.param_);

    if (numberOfPoints_ >= 0) {
        out << ",values=" ; pdiff(out, numberOfPoints_, other.numberOfPoints_);
    }

    out << ",format=";
    pdiff(out, format_, other.format_);

    if (!packing_.empty()) {
        out << ",packing=" ; pdiff(out, packing_, other.packing_);
    }

    if (!gridtype_.empty()) {
        out << ",gridtype=" ; pdiff(out, gridtype_, other.gridtype_);
    }

    if (!gridname_.empty()) {
        out << ",gridname="; pdiff(out, gridname_, other.gridname_);
    }

    if (resol_ >= 0) {
        out << ",resol="; pdiff(out, resol_, other.resol_);
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
        out << ",grid=" ; pdiff(out, north_south_, other.north_south_);
        out  << "/";
        pdiff(out, west_east_, other.west_east_);
    }

    if (area_) {
        out << ",area=" ;
        pdiff(out, north_, other.north_);
        out << "/" ;
        pdiff(out, west_, other.west_);
        out << "/";
        pdiff(out, south_, other.south_);
        out << "/";
        pdiff(out, east_, other.east_);
    }

    if (rotation_) {
        out << ",rotation=" ;
        pdiff(out, rotation_latitude_, other.rotation_latitude_);
        out << "/" ;
        pdiff(out, rotation_longitude_, other.rotation_longitude_);
    }

    for (auto j = values_.begin(); j != values_.end(); ++j) {
        out << "," << (*j).first << "=";

        auto k = other.values_.find((*j).first);

        if (k == other.values_.end()) {
            pdiff(out, (*j).second , std::string());
        }
        else {
            pdiff(out, (*j).second , (*k).second);
        }

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
    return numberOfPoints_;
}

bool GribField::match(const std::string& name, const std::string& value) const {
    auto j = values_.find(name);
    if (j != values_.end()) {
        return (*j).second == value;
    }

    if (name == "area") {
        std::ostringstream oss;
        if (area_) {
            oss << north_
                << '/'
                << west_
                << '/'
                << south_
                << '/'
                << east_;
        }
        return value == oss.str();
    }

    if (name == "grid") {
        std::ostringstream oss;
        if (grid_) {
            oss << west_east_
                << '/'
                << north_south_;
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
            oss << rotation_latitude_
                << '/'
                << rotation_longitude_;
        }
        return value == oss.str();
    }

    return false;
}


//----------------------------------------------------------------------------------------------------------------------

Field GribField::field(const char* buffer, size_t size,
                       const std::string& path, off_t offset,
                       const std::vector<std::string>& ignore) {

    GribField* field = new GribField(path, offset, size);
    Field result(field);

    grib_handle *h = grib_handle_new_from_message(0, buffer, size);
    ASSERT(h);
    HandleDeleter delh(h);

    static std::string gribToRequestNamespace = eckit::Resource<std::string>("gribToRequestNamespace", "mars");

    grib_keys_iterator *ks = grib_keys_iterator_new(h, GRIB_KEYS_ITERATOR_ALL_KEYS, gribToRequestNamespace.c_str());
    ASSERT(ks);
    GKeyIteratorDeleter delk(ks);

    /// @todo this code should be factored out into mir

    // bool sfc = false;

    std::map<std::string, std::string> req;

    while (grib_keys_iterator_next(ks)) {
        const char *name = grib_keys_iterator_get_name(ks);
        ASSERT(name);

        if (name[0] == '_') continue;
        if (::strcmp(name, "param") == 0) continue;

        char val[1024];
        size_t len = sizeof(val);

        GRIB_CALL( grib_keys_iterator_get_string(ks, val, &len) );

        field->insert(name, val);

        // if (::strcmp(val, "sfc") == 0) {
        //     sfc = true;
        // }

        req[name] = val;
    }


    long paramId;
    GRIB_CALL (grib_get_long(h, "paramId", &paramId));

    field->param(paramId);

    long numberOfDataPoints;
    GRIB_CALL (grib_get_long(h, "numberOfDataPoints", &numberOfDataPoints));
    field->numberOfPoints(numberOfDataPoints);

    // Look for request embbeded in GRIB message
    long local;

    if (grib_get_long(h, "localDefinitionNumber", &local) ==  0 && local == 191) {
        size_t size;
        /* TODO: Not grib2 compatible, but speed-up process */
        if (grib_get_size(h, "freeFormData", &size) ==  0 && size != 0) {
            unsigned char buffer[size];
            GRIB_CALL(grib_get_bytes(h, "freeFormData", buffer, &size) );

            eckit::MemoryStream s(buffer, size);

            int count;
            s >> count; // Number of requests
            ASSERT(count == 1);
            std::string tmp;
            s >> tmp; // verb
            s >> count;
            for (int i = 0; i < count; i++) {
                std::string keyword, value;
                int n;
                s >> keyword;
                std::transform(keyword.begin(), keyword.end(), keyword.begin(), tolower);
                s >> n; // Number of values
                ASSERT(n == 1);
                s >> value;
                std::transform(value.begin(), value.end(), value.begin(), tolower);
                field->insert(keyword, value);
                req[keyword] = value;
            }
        }
    }

    static eckit::Translator<long, std::string> l2s;


    {
        char value[1024];
        size_t len = sizeof(value);
        if (grib_get_string(h, "gridType", value, &len) == 0) {
            field->gridtype(value);


            if (strcmp(value, "regular_ll") == 0) {
                setGrid(*field, h);
                setArea(*field, h);
            } else  if (strcmp(value, "rotated_ll") == 0) {
                setGrid(*field, h);
                setArea(*field, h);
                {
                    double lat, lon;
                    GRIB_CALL(grib_get_double(h, "latitudeOfSouthernPoleInDegrees", &lat));
                    GRIB_CALL(grib_get_double(h, "longitudeOfSouthernPoleInDegrees", &lon) );
                    field->rotation(lat, lon);
                }
            }
            else if (strcmp(value, "sh") == 0) {

                // double d;
                {
                    long n = -1;
                    GRIB_CALL(grib_get_long(h, "pentagonalResolutionParameterJ", &n) );
                    field->resol(n);
                }
            }
            else if (strcmp(value, "reduced_gg") == 0) {
                {
                    long n = 0;
                    std::ostringstream oss;


                    GRIB_CALL(grib_get_long(h, "isOctahedral", &n) );

                    if (n) {
                        oss << "O";
                    }
                    else {

                        // Don't trust eccodes
                        size_t pl_size = 0;
                        GRIB_CALL(grib_get_size(h, "pl", &pl_size) );
                        long pl[pl_size];

                        GRIB_CALL(grib_get_long_array(h, "pl", pl, &pl_size) );

                        bool isOctahedral = true;
                        for (size_t i = 1 ; i < pl_size; i++) {
                            long diff = std::abs(pl[i] - pl[i - 1]);
                            if (diff != 4 && diff != 0) {
                                isOctahedral = false;
                                break;
                            }
                        }

                        if (isOctahedral) {
                            oss << "O";
                        }
                        else {
                            oss << "N";
                        }
                    }

                    GRIB_CALL(grib_get_long(h, "N", &n) );
                    oss << n;



                    // ASSERT(grib_get_double(h, "iDirectionIncrementInDegrees", &d) == 0);
                    // oss << '/' << rounded(d);
                    field->gridname(oss.str());
                }

                setArea(*field, h);
            } else if (strcmp(value, "regular_gg") == 0) {
                long n;
                {
                    std::ostringstream oss;


                    GRIB_CALL(grib_get_long(h, "N", &n) );
                    oss << "F" << n;

                    // ASSERT(grib_get_double(h, "iDirectionIncrementInDegrees", &d) == 0);
                    // oss << '/' << rounded(d);
                    field->gridname(oss.str());
                }
                setArea(*field, h);
            }
            else if (strcmp(value, "polar_stereographic") == 0) {
                eckit::Log::warning() << "Ignoring polar_stereographic in " << path << std::endl;
                return result;
            }
            else {
                std::ostringstream oss;
                oss << path << ": Unknown grid [" << value << "]";
                throw eckit::SeriousBug(oss.str());
            }
        }
    }



    // long scanningMode = 0;
    // if (grib_get_long(h, "scanningMode", &scanningMode) == 0) {
    //     field->insert("scanningMode", scanningMode);
    // }

    // long decimalScaleFactor = 0;
    // if (grib_get_long(h, "decimalScaleFactor", &decimalScaleFactor) == 0) {
    //     field->insert("decimalScaleFactor", decimalScaleFactor);
    // }


    long edition;
    if (grib_get_long(h, "edition", &edition) == 0) {
        field->format("grib" + l2s(edition));
    }

    long missingValuesPresent;
    if (grib_get_long(h, "missingValuesPresent", &missingValuesPresent) == 0) {
        if (missingValuesPresent) {
            field->missingValuesPresent(true);
        }
    }

    long bitsPerValue;
    if (grib_get_long(h, "bitsPerValue", &bitsPerValue) == 0) {
        field->accuracy(bitsPerValue);
    }

    long decimalScaleFactor;
    if (grib_get_long(h, "decimalScaleFactor", &decimalScaleFactor) == 0) {
        field->decimalScaleFactor(decimalScaleFactor);
    }


    {
        char value[1024];
        size_t len = sizeof(value);
        if (grib_get_string(h, "packingType", value, &len) == 0) {
            field->packing(value);
        }
    }

    {
        char value[1024];
        size_t len = sizeof(value);
        if (grib_get_string(h, "packing", value, &len) == 0) {
            field->packing(value);
        }
    }

    for (auto j = ignore.begin(); j != ignore.end(); ++j) {
        field->erase(*j);
    }

    return result;
}



void GribField::setArea(GribField& field, grib_handle *h) {
    double n = -99999, w = -99999, s = -99999, e = -99999;
    GRIB_CALL(grib_get_double(h, "latitudeOfFirstGridPointInDegrees", &n));
    GRIB_CALL(grib_get_double(h, "longitudeOfFirstGridPointInDegrees", &w));
    GRIB_CALL(grib_get_double(h, "latitudeOfLastGridPointInDegrees", &s));
    GRIB_CALL(grib_get_double(h, "longitudeOfLastGridPointInDegrees", &e));

    long scanningMode = 0;
    GRIB_CALL(grib_get_long(h, "scanningMode", &scanningMode));

    switch (scanningMode) {


    case 0:
        break;

    case 64:
        std::swap(n, s);
        break;

    default: {
        std::ostringstream oss;
        oss << "Invalid scanning mode " << scanningMode;
        throw eckit::SeriousBug(oss.str());
    }
    break;
    }


    field.area(n, w, s, e);
}



void GribField::setGrid(GribField& field, grib_handle *h) {


    double we = -99999, ns = -99999;
    GRIB_CALL(grib_get_double(h, "jDirectionIncrementInDegrees", &ns));
    GRIB_CALL(grib_get_double(h, "iDirectionIncrementInDegrees", &we));
    field.grid(ns, we);
}


//----------------------------------------------------------------------------------------------------------------------
}  // namespace compare

}  // namespace mir
