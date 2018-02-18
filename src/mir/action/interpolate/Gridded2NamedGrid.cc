/*
 * (C) Copyright 1996- ECMWF.
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

#include "mir/action/interpolate/Gridded2NamedGrid.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/namedgrids/NamedGrid.h"


namespace mir {
namespace action {


Gridded2NamedGrid::Gridded2NamedGrid(const param::MIRParametrisation& parametrisation):
    Gridded2GriddedInterpolation(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("gridname", gridname_));
}


Gridded2NamedGrid::~Gridded2NamedGrid() {
}

bool Gridded2NamedGrid::sameAs(const Action& other) const {
    const Gridded2NamedGrid* o = dynamic_cast<const Gridded2NamedGrid*>(&other);
    return o && (gridname_ == o->gridname_) && Gridded2GriddedInterpolation::sameAs(other);
}

void Gridded2NamedGrid::print(std::ostream& out) const {
    out << "Gridded2NamedGrid[gridname="
        << gridname_
        << ",";
    Gridded2GriddedInterpolation::print(out);
    out << "]";
}


const repres::Representation* Gridded2NamedGrid::outputRepresentation() const {
    const namedgrids::NamedGrid& ng = namedgrids::NamedGrid::lookup(gridname_);
    return ng.representation();
}

const char* Gridded2NamedGrid::name() const {
    return "Gridded2NamedGrid";
}



namespace {
static ActionBuilder< Gridded2NamedGrid > grid2grid("interpolate.grid2namedgrid");
}


}  // namespace action
}  // namespace mir

