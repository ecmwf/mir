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


#include "mir/netcdf/UnstructuredGrid.h"

#include <cmath>
#include <ostream>

#include "mir/netcdf/Dimension.h"
#include "mir/netcdf/Variable.h"
#include "mir/util/Exceptions.h"


namespace mir {
namespace netcdf {


UnstructuredGrid::UnstructuredGrid(const Variable& variable, const std::vector<double>& latitudes,
                                   const std::vector<double>& longitudes) :
    GridSpec(variable), latitudes_(latitudes), longitudes_(longitudes) {
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


UnstructuredGrid::~UnstructuredGrid() = default;


void UnstructuredGrid::print(std::ostream& s) const {
    s << "UnstructuredGrid[points=" << latitudes_.size() << "]";
}


bool UnstructuredGrid::has(const std::string& name) const {
    // Log::info() << "has " << name << std::endl;

    // Note: only "gridded" is supported
    return (name == "gridded");
}


bool UnstructuredGrid::get(const std::string& name, std::vector<double>& values) const {
    // Log::info() << "get " << name << std::endl;

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


bool UnstructuredGrid::get(const std::string& /*name*/, long& /*value*/) const {
    // Log::info() << "get " << name << std::endl;

    // Log::info() << "UnstructuredGrid::get " << name << " failed" << std::endl;

    return false;
}


bool UnstructuredGrid::get(const std::string& name, std::string& value) const {
    // Log::info() << "get " << name << std::endl;
    if (name == "gridType") {
        value = "unstructured_grid";
        return true;
    }

    // Log::info() << "UnstructuredGrid::get " << name << " failed" << std::endl;


    return false;
}


bool UnstructuredGrid::get(const std::string& name, double& value) const {

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

    // Log::info() << "UnstructuredGrid::get " << name << " failed" << std::endl;

    return false;
}


static bool check_axis(const Variable& variable, const Variable& axis, std::vector<double>& v) {

    auto axis_dimensions     = axis.dimensions();
    auto variable_dimensions = variable.dimensions();

    if (axis_dimensions.size() != 2 || variable_dimensions.size() < 2) {
        return false;
    }

    Dimension* vars[] = {variable_dimensions[variable_dimensions.size() - 2], variable_dimensions.back()};
    if (vars[0] != axis_dimensions[0] || vars[1] != axis_dimensions[1]) {
        return false;
    }

    axis.values(v);
    return true;
}


GridSpec* UnstructuredGrid::guess(const Variable& variable, const Variable& latitudes, const Variable& longitudes) {

    if (variable.numberOfDimensions() < 2) {
        return nullptr;
    }

    std::vector<double> lats;
    if (!check_axis(variable, latitudes, lats)) {
        return nullptr;
    }

    std::vector<double> lons;
    if (!check_axis(variable, longitudes, lons)) {
        return nullptr;
    }

    return new UnstructuredGrid(variable, lats, lons);
}


void UnstructuredGrid::reorder(MIRValuesVector& /*values*/) const {
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


static GridSpecGuesserBuilder<UnstructuredGrid> builder(99);


}  // namespace netcdf
}  // namespace mir
