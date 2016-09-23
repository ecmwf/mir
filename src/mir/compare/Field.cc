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


namespace mir {
namespace compare {

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
    resol_(0) {}

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

double Field::compare(const Field& other) const {
    return compareAreas(other);
}


double Field::compareAreas(const Field& other) const {

    if (!area_ || !other.area_) {
        return -1;
    }

    double w1 = west_;
    double e1 = east_;
    double n1 = north_;
    double s1 = south_;

    double w2 = other.west_;
    double e2 = other.east_;
    double n2 = other.north_;
    double s2 = other.south_;

    if (s1 <= -78) {
        auto j = values_.find("param");
        if (j != values_.end()) {
            auto s = (*j).second;
            if (s.length() == 6 && s.substr(0, 3) == "140") {
                s1 = -90;
            }
        }
    }

    if (s2 <= -78) {
        auto j = other.values_.find("param");
        if (j != other.values_.end()) {
            auto s = (*j).second;
            if (s.length() == 6 && s.substr(0, 3) == "140") {
                s2 = -90;
            }
        }
    }



    while (w1 >= e1) {
        w1 -= 360;
    }

    while (w2 >= e2) {
        w2 -= 360;
    }

    while (w1 < 0) {
        w1 += 360;
        e1 += 360;
    }

    while (w2 < 0) {
        w2 += 360;
        e2 += 360;
    }

    double ww = std::min(w1, w2);

    while (w1 >= ww + 360) {
        w1 -= 360;
        e1 -= 360;
    }

    while (w2 >= ww + 360) {
        w2 -= 360;
        e2 -= 360;
    }

//==========

    // Union
    double un = std::max(n1, n2);
    double us = std::min(s1, s2);
    double ue = std::max(e1, e2);
    double uw = std::min(w1, w2);

    double uarea = (un - us) * (ue - uw);


    if ((un - us) < 0 || (ue - uw) < 0) {
        std::ostringstream oss;
        oss << "Cannot compute union: "
            << n1 << "/" << w1 << "/" << s1 << "/" << e1 << " and "
            << n2 << "/" << w2 << "/" << s2 << "/" << e2 << " ==> "
            << un << "/" << uw << "/" << us << "/" << ue;
        oss << std::endl << *this;
        oss << std::endl << other;
        throw eckit::SeriousBug(oss.str());

    }


    // Intersction
    double in = std::min(n1, n2);
    double is = std::max(s1, s2);
    double ie = std::min(e1, e2);
    double iw = std::max(w1, w2);

    double iarea = (in - is) * (ie - iw);

    if (uarea == 0) {


        if (un == us) {
            ASSERT(ue > uw);

            if (in != is) {
                return 0;
            }

            uarea = ue - uw;
            iarea = ie - iw;
        }

        if (ue == uw) {
            ASSERT(un != us);

            if (ie != iw) {
                return 0;
            }

            uarea = un - us;
            iarea = in - is;
        }

        ASSERT(uarea > 0);

    }

    if ((in - is) < 0 || (ie - iw) < 0) {
        iarea = 0;
    }

    return iarea / uarea;

}


bool Field::sameArea(const Field& other) const {

    if (!area_ && !other.area_)
        return true;

    if (area_ != other.area_)
        return false;

    return compareAreas(other) > 0.7; // Observed for N320
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

    if (packing_ == "grid_simple_matrix" && other.packing_ == "grid_simple") {
        return true;
    }

    if (packing_ == "grid_simple" && other.packing_ == "grid_simple_matrix") {
        return true;
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
    return true;

    return gridtype_ == other.gridtype_;
}

bool Field::sameFormat(const Field& other) const {
    return format_ == other.format_;
}

bool Field::sameParam(const Field& other) const {


    if (param_ == 139 && other.param_ == 228095) {
        return true;
    }
    if (param_ == 228095 && other.param_ == 139) {
        return true;
    }


    return param_ == other.param_;
}

bool Field::sameAccuracy(const Field& other) const {
    if (accuracy_ == 0 || other.accuracy_ == 0) {
        return true;
    }
    return accuracy_  == other.accuracy_;
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


static double normalize(double longitude) {
    while (longitude < 0) {
        longitude += 360;
    }
    while (longitude >= 360) {
        longitude -= 360;
    }
    return longitude;
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

        if (west_ < other.west_) {
            return true;
        }

        if (west_ > other.west_) {
            return false;
        }

        if (south_ < other.south_) {
            return true;
        }

        if (south_ > other.south_) {
            return false;
        }

        if (east_ < other.east_) {
            return true;
        }

        if (east_ > other.east_) {
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

        if (rotation_longitude_ < other.rotation_longitude_) {
            return true;
        }

        if (rotation_longitude_ > other.rotation_longitude_) {
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

    if (resol_) {
        out << ",resol=" << resol_;
    }

    if (accuracy_ >= 0) {
        out << ",accuracy=" << accuracy_;
    }

    if (bitmap_) {
        out << ",bitmap=yes";
    }

    if (grid_) {
        out << ",grid=" << north_south_ << "/" << west_east_;
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
    return (values_ == other.values_) ;
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
        out << eckit::Colour::red << eckit::Colour::bold << v1 << eckit::Colour::reset;
    }
    else {
        out << v1;
    }
}

std::ostream& Field::printDifference(std::ostream & out, const Field & other) const {

    out << "[param=";
    pdiff(out, param_, other.param_);

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

    if (resol_) {
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
    // out << " - " << info_;
    out << "]";

    return out;
}


//----------------------------------------------------------------------------------------------------------------------


//----------------------------------------------------------------------------------------------------------------------
}  // namespace compare

}  // namespace mir
