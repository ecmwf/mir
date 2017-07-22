/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// Baudouin Raoult - ECMWF Jan 2015

#include "mir/netcdf/Curvilinear.h"
#include "mir/netcdf/Variable.h"
#include "eckit/types/Types.h"
#include "mir/netcdf/Dimension.h"
#include "mir/netcdf/HyperCube.h"

#include <iostream>

namespace mir {
namespace netcdf {

Curvilinear::Curvilinear(const Variable &variable,
                         const std::vector<double>& latitudes,
                         const std::vector<double>& longitudes):
    GridSpec(variable),
    latitudes_(latitudes),
    longitudes_(longitudes)
{
    ASSERT(latitudes_.size() == longitudes_.size());
    ASSERT(latitudes_.size() >= 2);

    north_ = south_ = latitudes_[0];
    for (double d : latitudes_) {
        north_ = std::max(north_, d);
        south_ = std::min(south_, d);
    }

    east_ = west_ = longitudes_[0];
    for (double d : longitudes_) {
        east_ = std::max(east_, d);
        west_ = std::min(west_, d);
    }
}

Curvilinear::~Curvilinear()
{
}


void Curvilinear::print(std::ostream& s) const
{
    s << "Curvilinear[points=" << latitudes_.size() << "]";
}

bool Curvilinear::has(const std::string& name) const {
    // std::cout << "has " << name << std::endl;
    if (name == "gridded") {
        return true;
    }

    // std::cout << "Curvilinear::has " << name << " failed" << std::endl;


    return false;
}

bool Curvilinear::get(const std::string &name, std::vector<double> &values) const {
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

bool Curvilinear::get(const std::string&name, long& value) const {
    // std::cout << "get " << name << std::endl;

    // std::cout << "Curvilinear::get " << name << " failed" << std::endl;

    return false;
}

bool Curvilinear::get(const std::string&name, std::string& value) const {
    // std::cout << "get " << name << std::endl;
    if (name == "gridType") {
        value = "unstructured_grid";
        return true;
    }

    // std::cout << "Curvilinear::get " << name << " failed" << std::endl;


    return false;
}

bool Curvilinear::get(const std::string &name, double &value) const {

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



    // std::cout << "Curvilinear::get " << name << " failed" << std::endl;


    return false;
}


//================================================================

static bool check_axis(const Variable &variable, const Variable & axis, std::vector<double>& v) {



    auto axis_dimensions = axis.dimensions();
    auto variable_dimensions = variable.dimensions();

    if (axis_dimensions.size() != 2) {
        return false;
    }

    if (variable_dimensions[variable_dimensions.size() - 2 ] != axis_dimensions[0]) {
        return false;
    }

    if (variable_dimensions[variable_dimensions.size() - 1 ] != axis_dimensions[1]) {
        return false;
    }


    axis.values(v);

    return true;
}

GridSpec* Curvilinear::guess(const Variable &variable,
                             const Variable &latitudes,
                             const Variable &longitudes) {

    if (variable.numberOfDimensions() < 2)
        return 0;
}

std::vector<double> lats;
if (!check_axis(variable, latitudes, lats)) {
    return 0;
}

double west, east;
std::vector<double> lons;
if (!check_axis(variable, longitudes, lons)) {
    return 0;
}

//

auto dimensions = lats.dimensions();
ASSERT(dimensions.size() == 2);


struct Index {
    size_t ni_;
    size_t nj_;

    size_t operator()(size_t i, size_t j) { return j + i * nj_; }
};

Index index;
index_.ni_ = dimensions[0]->count();
index_.nj_ = dimensions[1]->count();



std::cout << "Curvilinear " << index.ni_ << " " << index.nj_ << std::endl;
for (size_t i = 0; i < index.ni_; i++) {
    for (size_t j = 0; j < index.nj_; j++) {

        std::cout << lats[index(i, j)] << " " << lons[index(i, j)] << std::endl;

    }
}

return new Curvilinear(variable, lats, lons);

}


void Curvilinear::reorder(std::vector<double>& values) const {
    // size_t ni = latitudes_.size();
    // size_t nj = longitudes_.size();

    // if (jScansPositively_) {
    //   ASSERT(values.size() == ni * nj);

    //   std::vector<double> out(values.size());

    //   size_t count = 0;
    //   for (int j = nj - 1 ; j >= 0; --j) {
    //     for (size_t i = 0 ; i <  ni; ++i) {
    //       out[count++] = values[j * ni + i];
    //     }
    //   }
    //   ASSERT(count == out.size());
    //   std::swap(values, out);
    //   return;
    // }
}


static GridSpecGuesserBuilder<Curvilinear> builder(3);
}
}
