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

#include "mir/action/transform/Sh2NamedGrid.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/namedgrids/NamedGrid.h"

namespace mir {
namespace action {


Sh2NamedGrid::Sh2NamedGrid(const param::MIRParametrisation& parametrisation):
    Sh2GriddedTransform(parametrisation) {

    ASSERT(parametrisation_.get("user.gridname", gridname_));

}


Sh2NamedGrid::~Sh2NamedGrid() {
}


void Sh2NamedGrid::print(std::ostream& out) const {
    out << "Sh2NamedGrid[gridname=" << gridname_ << "]";
}


const repres::Representation* Sh2NamedGrid::outputRepresentation(const repres::Representation* inputRepres) const {
    const namedgrids::NamedGrid& ng = namedgrids::NamedGrid::lookup(gridname_);
    return ng.outputRepresentation(parametrisation_, inputRepres);
}


namespace {
static ActionBuilder< Sh2NamedGrid > grid2grid("transform.sh2namedgrid");
}


}  // namespace action
}  // namespace mir

