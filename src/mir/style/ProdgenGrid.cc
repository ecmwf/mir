/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "mir/style/ProdgenGrid.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace style {



ProdgenGrid::ProdgenGrid(const param::MIRParametrisation &parametrisation):
    parametrisation_(parametrisation) {
}


ProdgenGrid::~ProdgenGrid() {
}


bool ProdgenGrid::get(const std::string &name, std::vector<double>& value) const {
    eckit::Log::debug<LibMir>() << "ProdgenGrid::get(" << name << ")" << std::endl;
    ASSERT(name == "grid"); // For now

    value.resize(2);

    std::vector<double> grid;

    ASSERT(parametrisation_.get("user.grid", grid));
    ASSERT(grid.size() == 2);

    double x = grid[0] * 10;
    double y = grid[1] * 10;

    if(size_t(x) == x && size_t(y) == y) {
        value[0] = 1./10.;
        value[1] = 1./10.;
        return true;
    }

    value[0] = 1./8.;
    value[1] = 1./8.;

    // eckit::Log::debug<LibMir>() << "ProdgenGrid: N is " << N << ", selecting reduced N" << value << std::endl;
    return true;
}


void ProdgenGrid::print(std::ostream &out) const {
    out << "<ProdgenGrid>";
}


}  // namespace param
}  // namespace mir

