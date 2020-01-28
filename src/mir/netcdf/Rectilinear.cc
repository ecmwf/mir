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


#include "mir/netcdf/Rectilinear.h"
#include "eckit/types/Types.h"
#include "mir/netcdf/Variable.h"

#include <iostream>

namespace mir {
namespace netcdf {

Rectilinear::Rectilinear(const Variable& variable, double north, double south, const std::vector<double>& latitudes,
                         double west, double east, const std::vector<double>& longitudes) :
    GridSpec(variable),
    jScansPositively_(false),
    north_(north),
    south_(south),
    latitudes_(latitudes),
    west_(west),
    east_(east),
    longitudes_(longitudes) {

    if (north_ < south_) {
        std::swap(north_, south_);
    }

    if (east_ < west_) {
        std::swap(east_, west_);
    }
}

Rectilinear::~Rectilinear() = default;


void Rectilinear::print(std::ostream& s) const {
    s << "Rectilinear[bbox=" << north_ << "/" << west_ << "/" << south_ << "/" << east_ << "]";
}

bool Rectilinear::has(const std::string& name) const {
    // std::cout << "has " << name << std::endl;
    if (name == "gridded") {
        return true;
    }

    // std::cout << "Rectilinear::has " << name << " failed" << std::endl;


    return false;
}

bool Rectilinear::get(const std::string& name, std::vector<double>& values) const {
    // std::cout << "get " << name << std::endl;

    if (name == "latitudes") {
        values = latitudes_;
        return true;
    }

    if (name == "longitudes") {
        values = longitudes_;
        return true;
    }

    return false;
}

bool Rectilinear::get(const std::string& name, long& value) const {
    // std::cout << "get " << name << std::endl;

    // std::cout << "Rectilinear::get " << name << " failed" << std::endl;

    return false;
}

bool Rectilinear::get(const std::string& name, std::string& value) const {
    // std::cout << "get " << name << std::endl;
    if (name == "gridType") {
        value = "irregular_latlon";
        return true;
    }

    // std::cout << "Rectilinear::get " << name << " failed" << std::endl;


    return false;
}

bool Rectilinear::get(const std::string& name, double& value) const {

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


    // std::cout << "Rectilinear::get " << name << " failed" << std::endl;


    return false;
}


//================================================================

static bool check_axis(const Variable& axis, double& first, double& last, std::vector<double>& v) {

    if (axis.numberOfDimensions() != 1) {
        return false;
    }

    axis.values(v);

    if (v.size() < 2) {
        return false;
    }


    first = v[0];
    last  = v[v.size() - 1];

    return true;
}

GridSpec* Rectilinear::guess(const Variable& variable, const Variable& latitudes, const Variable& longitudes) {

    double north, south;
    std::vector<double> lats;
    if (!check_axis(latitudes, north, south, lats)) {
        return 0;
    }

    double west, east;
    std::vector<double> lons;
    if (!check_axis(longitudes, west, east, lons)) {
        return 0;
    }


    return new Rectilinear(variable, north, south, lats, west, east, lons);
}


void Rectilinear::reorder(MIRValuesVector& values) const {
    size_t ni = latitudes_.size();
    size_t nj = longitudes_.size();

    if (jScansPositively_) {
        ASSERT(values.size() == ni * nj);

        MIRValuesVector out(values.size());

        size_t count = 0;
        for (int j = nj - 1; j >= 0; --j) {
            for (size_t i = 0; i < ni; ++i) {
                out[count++] = values[j * ni + i];
            }
        }
        ASSERT(count == out.size());
        std::swap(values, out);
        return;
    }
}


static GridSpecGuesserBuilder<Rectilinear> builder(1);


}  // namespace netcdf
}  // namespace mir
