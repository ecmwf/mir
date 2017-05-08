/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include <iostream>
#include <cmath>

#include "mir/compare/Field.h"
#include "mir/compare/FieldSet.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/log/Colour.h"

#include "eckit/option/CmdArgs.h"
#include "eckit/option/SimpleOption.h"


namespace mir {
namespace compare {


static bool normaliseLongitudes_ = false;
static bool ignoreAccuracy_ = false;
static bool ignorePacking_ = false;


static double areaPrecisionN_ = 0.;
static double areaPrecisionW_ = 0.;
static double areaPrecisionS_ = 0.;
static double areaPrecisionE_ = 0.;


void Field::addOptions(std::vector<eckit::option::Option*>& options) {
    using namespace eckit::option;

    options.push_back(new SimpleOption<bool>("normalise-longitudes",
                      "Normalise longitudes between 0 and 360"));

    options.push_back(new SimpleOption<double>("compare-areas-threshold",
                      "Threshold when comparing areas with Jaccard distance"));

    options.push_back(new SimpleOption<double>("value-count-comparaison-threshold",
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

}


static double normalize(double longitude) {

    if (!normaliseLongitudes_) {
        return longitude;
    }

    while (longitude < 0) {
        longitude += 360;
    }
    while (longitude >= 360) {
        longitude -= 360;
    }
    return longitude;
}


void Field::setOptions(const eckit::option::CmdArgs &args) {
    args.get("normalise-longitudes", normaliseLongitudes_);
    args.get("ignore-accuracy", ignoreAccuracy_);
    args.get("ignore-packing", ignorePacking_);
    args.get("area-precision-north", areaPrecisionN_);
    args.get("area-precision-west", areaPrecisionW_);
    args.get("area-precision-south", areaPrecisionS_);
    args.get("area-precision-east", areaPrecisionE_);

}



Field::Field(const std::string& path, off_t offset, size_t length):
    info_(path, offset, length),
    param_(-1),
    area_(false),
    north_(0),
    west_(0),
    south_(0),
    east_(0),
    accuracy_(-1),
    grid_(false),
    west_east_(0),
    north_south_(0),
    rotation_(false),
    rotation_latitude_(0),
    rotation_longitude_(0),
    bitmap_(false),
    resol_(-1),
    numberOfPoints_(-1) {}

void Field::insert(const std::string& key, const std::string& value) {
    values_[key] = value;
}

void Field::insert(const std::string& key, long value) {
    std::ostringstream oss;
    oss << value;
    insert(key, oss.str());
}


void Field::erase(const std::string& key) {
    values_.erase(key);
}

off_t Field::offset() const {
    return info_.offset();
}

size_t Field::length() const {
    return info_.length();
}

const std::string& Field::path() const {
    return info_.path();
}

// double Field::compare(const Field& other) const {
//     return compareAreas(other);
// }


void Field::compareAreas(std::ostream& out, const Field& other) const {

    if (!area_ || !other.area_) {
        return;
    }

    double w1 = normalize(west_);
    double e1 = normalize(east_);
    double n1 = north_;
    double s1 = south_;

    double w2 = normalize(other.west_);
    double e2 = normalize(other.east_);
    double n2 = other.north_;
    double s2 = other.south_;

    out << ::fabs(n1 - n2) << '/' << ::fabs(w1 - w2) << '/' << ::fabs(s1 - s2) << '/' << ::fabs(e1 - e2);

}


inline bool sameLatLon(double a, double b, double e) {
    return ::fabs(a - b) <= e;
}


bool Field::sameArea(const Field& other) const {

    if (!area_ && !other.area_)
        return true;

    if (area_ != other.area_)
        return false;

    double w1 = normalize(west_);
    double e1 = normalize(east_);
    double n1 = north_;
    double s1 = south_;

    double w2 = normalize(other.west_);
    double e2 = normalize(other.east_);
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

    // return compareAreas(other) > areaComparaisonThreshold_;
}


void Field::bitmap(bool on)  {
    bitmap_ = on;
}

void Field::resol(size_t resol)  {
    resol_ = resol;
}

void Field::param(long param)  {
    param_ = param;
}

void Field::numberOfPoints(long n)  {
    numberOfPoints_ = n;
}

void Field::gridname(const std::string& name)  {
    gridname_ = name;
}

void Field::format(const std::string& format)  {
    format_ = format;
}

void Field::gridtype(const std::string& type)  {
    gridtype_ = type;
}

bool Field::samePacking(const Field& other) const {

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

bool Field::sameResol(const Field& other) const {
    return resol_ == other.resol_;
}

bool Field::sameGridname(const Field& other) const {
    return gridname_ == other.gridname_;
}

bool Field::sameGridtype(const Field& other) const {
    return gridtype_ == other.gridtype_;
}

bool Field::sameFormat(const Field& other) const {
    return format_ == other.format_;
}

bool Field::sameParam(const Field& other) const {
    return param_ == other.param_;
}

bool Field::sameAccuracy(const Field& other) const {

    if (ignoreAccuracy_) {
        return true;
    }


    if (accuracy_ == 0 || other.accuracy_ == 0) {
        return true;
    }
    return accuracy_  == other.accuracy_;
}

bool Field::sameNumberOfPoints(const Field& other) const {
    return numberOfPoints_ == other.numberOfPoints_;
}

bool Field::sameBitmap(const Field& other) const {
    return true;
}

bool Field::sameGrid(const Field& other) const {

    if (grid_ != other.grid_) {
        return false;
    }

    if (grid_) {

        return (north_south_ == other.north_south_) &&
               (west_east_ == other.west_east_) ;
    }

    return true;
}


bool Field::sameRotation(const Field& other) const {

    if (rotation_ != other.rotation_) {
        return false;
    }

    if (rotation_) {

        return (rotation_latitude_ == other.rotation_latitude_) &&
               (normalize(rotation_longitude_) == normalize(other.rotation_longitude_)) ;
    }

    return true;
}

bool Field::operator<(const Field & other) const {

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

    if (bitmap_ < other.bitmap_) {
        return true;
    }

    if (bitmap_ > other.bitmap_) {
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

        if (normalize(west_) < normalize(other.west_)) {
            return true;
        }

        if (normalize(west_) > normalize(other.west_)) {
            return false;
        }

        if (south_ < other.south_) {
            return true;
        }

        if (south_ > other.south_) {
            return false;
        }

        if (normalize(east_) < normalize(other.east_)) {
            return true;
        }

        if (normalize(east_) > normalize(other.east_)) {
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

        if (normalize(rotation_longitude_) < normalize(other.rotation_longitude_)) {
            return true;
        }

        if (normalize(rotation_longitude_) > normalize(other.rotation_longitude_)) {
            return false;
        }

    }

    return values_ < other.values_;

}

std::map<std::string, std::string>::const_iterator Field::begin() const {
    return values_.begin();
}

std::map<std::string, std::string>::const_iterator Field::end() const {
    return values_.end();
}

std::map<std::string, std::string>::const_iterator Field::find(const std::string & key) const {
    return values_.find(key);
}


void Field::area(double n, double w, double s, double e) {
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

void Field::accuracy(long n) {
    accuracy_ = n;
}

void Field::packing(const std::string & packing) {
    packing_ = packing;
}


void Field::grid(double ns, double we) {
    grid_ = true;
    north_south_ = ns;
    west_east_ = we;
}

void Field::rotation(double lat, double lon) {
    rotation_ = true;
    rotation_latitude_ = lat;
    rotation_longitude_ = lon;
}

void Field::print(std::ostream & out) const {

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

    if (bitmap_) {
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

bool Field::sameField(const Field & other) const {
    return values_ == other.values_;
}


bool Field::match(const Field & other) const {
    return sameParam(other) && sameField(other);
    // &&
    // (sameGrid(other)  || sameResol(other) || sameGridname(other))   ;
    // sameRotation(other) &&
    // sameGridname(other) &&
    // sameGridtype(other) &&
    // sameResol(other) &&
    // (compareAreas(other) > 0.1);
}


bool Field::same(const Field & other) const {
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



std::vector<Field> Field::bestMatches(const FieldSet & fields) const {
    std::vector<Field> matches;

    for (auto k = fields.begin(); k != fields.end(); ++k) {
        const auto& other = *k;

        if (match(other)) {
            matches.push_back(other);
        }

    }

    return matches;

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

std::ostream& Field::printDifference(std::ostream & out, const Field & other) const {

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

    if (accuracy_ >= 0) {
        out << ",accuracy=";
        pdiff(out, accuracy_, other.accuracy_);
    }

    if (bitmap_) {
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


bool Field::wrapped() const {
    if (!area_) {
        return false;
    }

    double w = normalize(west_);
    double e = normalize(east_);

    return w == e;
}

bool Field::match(const std::string& name, const std::string& value) const {
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

    if (name == "bitmap") {
        std::ostringstream oss;
        oss << (bitmap_ ? "yes" : "no");
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


//----------------------------------------------------------------------------------------------------------------------
}  // namespace compare

}  // namespace mir
