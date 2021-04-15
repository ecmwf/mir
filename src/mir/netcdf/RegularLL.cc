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


#include "mir/netcdf/RegularLL.h"

#include <ostream>

#include "mir/netcdf/Variable.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace netcdf {


RegularLL::RegularLL(const Variable& variable, double north, double south, double south_north_increment, double west,
                     double east, double west_east_increment) :
    GridSpec(variable),
    north_(north),
    south_(south),
    south_north_increments_(south_north_increment),
    west_(west),
    east_(east),
    west_east_increment_(west_east_increment),
    jScansPositively_(false) {

    if (north_ < south_) {
        std::swap(north_, south_);
        jScansPositively_ = true;
    }

    ASSERT(east_ > west_);
    ASSERT(south_north_increments_ > 0);
    ASSERT(west_east_increment_ > 0);

    // TODO: use Fractions

    nj_ = size_t((north_ - south_) / south_north_increments_ + 1);
    ni_ = size_t((east_ - west_) / west_east_increment_ + 1);
}


RegularLL::~RegularLL() = default;


void RegularLL::print(std::ostream& s) const {
    s << "RegularLL[bbox=" << north_ << "/" << west_ << "/" << south_ << "/" << east_
      << ",grid=" << west_east_increment_ << "/" << south_north_increments_ << ",ni=" << ni_ << ",nj=" << nj_ << "]";
}


bool RegularLL::has(const std::string& name) const {
    // Log::info() << "has " << name << std::endl;

    // Note: only "gridded" is supported
    return (name == "gridded");
}


bool RegularLL::get(const std::string& name, long& value) const {
    // Log::info() << "get " << name << std::endl;

    if (name == "Nj") {
        value = long(nj_);
        return true;
    }

    if (name == "Ni") {
        value = long(ni_);
        return true;
    }

    // Log::info() << "RegularLL::get " << name << " failed" << std::endl;

    return false;
}


bool RegularLL::get(const std::string& name, std::string& value) const {
    // Log::info() << "get " << name << std::endl;
    if (name == "gridType") {
        value = "regular_ll";
        return true;
    }

    // Log::info() << "RegularLL::get " << name << " failed" << std::endl;

    return false;
}


bool RegularLL::get(const std::string& /*name*/, std::vector<double>& /*value*/) const {
    return false;
}


bool RegularLL::get(const std::string& name, double& value) const {

    if (name == "north") {
        value = north_;
        return true;
    }

    if (name == "south") {
        value = south_;
        return true;
    }

    if (name == "west") {
        value = west_;
        return true;
    }

    if (name == "east") {
        value = east_;
        return true;
    }

    if (name == "south_north_increment") {
        value = south_north_increments_;
        return true;
    }

    if (name == "west_east_increment") {
        value = west_east_increment_;
        return true;
    }

    // Log::info() << "RegularLL::get " << name << " failed" << std::endl;


    return false;
}


static bool check_axis(const Variable& axis, double& first, double& last, double& increment) {

    if (axis.numberOfDimensions() != 1) {
        return false;
    }

    std::vector<double> v;
    axis.values(v);

    if (v.size() < 2) {
        return false;
    }

    double d = v[1] - v[0];

    for (size_t i = 1; i < v.size(); ++i) {
        if ((v[i] - v[i - 1]) != d) {
            return false;
        }
    }

    first     = v.front();
    last      = v.back();
    increment = d > 0 ? d : -d;

    return true;
}


GridSpec* RegularLL::guess(const Variable& variable, const Variable& latitudes, const Variable& longitudes) {

    double north;
    double south;
    double south_north_increment;
    if (!check_axis(latitudes, north, south, south_north_increment)) {
        return nullptr;
    }

    double west;
    double east;
    double west_east_increment;
    if (!check_axis(longitudes, west, east, west_east_increment)) {
        return nullptr;
    }

    return new RegularLL(variable, north, south, south_north_increment, west, east, west_east_increment);
}


void RegularLL::reorder(MIRValuesVector& values) const {
    if (jScansPositively_) {
        ASSERT(values.size() == ni_ * nj_);

        MIRValuesVector out(values.size());

        size_t count = 0;
        for (int j = int(nj_) - 1; j >= 0; --j) {
            auto ju = size_t(j);
            for (size_t i = 0; i < ni_; ++i) {
                out[count++] = values[ju * ni_ + i];
            }
        }
        ASSERT(count == out.size());
        std::swap(values, out);
        return;
    }
}


static GridSpecGuesserBuilder<RegularLL> builder(0);  // First choice


}  // namespace netcdf
}  // namespace mir
