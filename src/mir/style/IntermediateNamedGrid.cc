/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @date May 2017


#include "mir/style/IntermediateNamedGrid.h"

#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace style {


namespace {
static IntermediateGridBuilder<IntermediateNamedGrid> __intermediate_grid_gridname("gridname");
}


IntermediateNamedGrid::IntermediateNamedGrid(const param::MIRParametrisation& parametrisation) :
    IntermediateGrid(parametrisation) {

    parametrisation_.get("spectral-intermediate-grid", gridname_);
    ASSERT(gridname_.length());
}


void IntermediateNamedGrid::print(std::ostream &out) const {
    out << "IntermediateNamedGrid[gridname=" << gridname_ << "]";
}


std::string IntermediateNamedGrid::getGridname() const {
    return gridname_;
}


}  // namespace style
}  // namespace mir

