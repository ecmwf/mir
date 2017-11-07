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

#include "mir/action/interpolate/Gridded2GridDef.h"

#include <iostream>

#include "eckit/filesystem/PathName.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/other/UnstructuredGrid.h"


namespace mir {
namespace action {


Gridded2GridDef::Gridded2GridDef(const param::MIRParametrisation& parametrisation):
    Gridded2GriddedInterpolation(parametrisation) {
    ASSERT(parametrisation_.user().get("griddef", griddef_));
}


Gridded2GridDef::~Gridded2GridDef() {
}

bool Gridded2GridDef::sameAs(const Action& other) const {
    const Gridded2GridDef* o = dynamic_cast<const Gridded2GridDef*>(&other);
    return o && (griddef_ == o->griddef_);
}

void Gridded2GridDef::print(std::ostream& out) const {
    out << "Gridded2GridDef[griddef=" << griddef_ << "]";
}


const repres::Representation* Gridded2GridDef::outputRepresentation() const {
    return new repres::other::UnstructuredGrid(griddef_);
}


namespace {
static ActionBuilder< Gridded2GridDef > grid2grid("interpolate.grid2griddef");
}


}  // namespace action
}  // namespace mir

