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


#include "mir/action/interpolate/Gridded2RotatedNamedGrid.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/namedgrids/NamedGrid.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace action {


Gridded2RotatedNamedGrid::Gridded2RotatedNamedGrid(const param::MIRParametrisation &parametrisation):
    Gridded2GriddedInterpolation(parametrisation) {


    ASSERT(parametrisation_.user().get("gridname", gridname_));
    std::vector<double> value;

    ASSERT(parametrisation_.user().get("rotation", value));
    ASSERT(value.size() == 2);

    rotation_ = util::Rotation(value[0], value[1]);
}


Gridded2RotatedNamedGrid::~Gridded2RotatedNamedGrid() {
}


bool Gridded2RotatedNamedGrid::sameAs(const Action& other) const {
    const Gridded2RotatedNamedGrid* o = dynamic_cast<const Gridded2RotatedNamedGrid*>(&other);
    return o && (gridname_ == o->gridname_) && (rotation_ == o->rotation_);
}

void Gridded2RotatedNamedGrid::print(std::ostream &out) const {
    out << "Gridded2RotatedNamedGrid[gridname=" << gridname_ << ",rotation=" << rotation_ << "]";
}

const repres::Representation *Gridded2RotatedNamedGrid::outputRepresentation() const {
    const namedgrids::NamedGrid &ng = namedgrids::NamedGrid::lookup(gridname_);
    return ng.representation(rotation_);
}


namespace {
static ActionBuilder< Gridded2RotatedNamedGrid > grid2grid("interpolate.grid2rotated-namedgrid");
}


}  // namespace action
}  // namespace mir

