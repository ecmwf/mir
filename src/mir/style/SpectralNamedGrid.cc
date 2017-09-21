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


#include "mir/style/SpectralNamedGrid.h"

#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace style {


namespace {
static SpectralGridBuilder<SpectralNamedGrid> __spectral_gridname("gridname");
}


SpectralNamedGrid::SpectralNamedGrid(const param::MIRParametrisation& parametrisation) :
    SpectralGrid(parametrisation) {

    parametrisation_.get("spectral-grid", gridname_);
    ASSERT(gridname_.length());
}


void SpectralNamedGrid::print(std::ostream &out) const {
    out << "SpectralNamedGrid[gridname=" << gridname_ << "]";
}


std::string SpectralNamedGrid::getGridname() const {
    return gridname_;
}


}  // namespace style
}  // namespace mir

