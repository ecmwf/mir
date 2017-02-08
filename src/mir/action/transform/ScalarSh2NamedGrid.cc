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


#include "mir/action/transform/ScalarSh2NamedGrid.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/namedgrids/NamedGrid.h"

namespace mir {
namespace action {
namespace transform {


ScalarSh2NamedGrid::ScalarSh2NamedGrid(const param::MIRParametrisation& parametrisation):
    Sh2Gridded(parametrisation) {

    ASSERT(parametrisation_.get("user.gridname", gridname_));

}


ScalarSh2NamedGrid::~ScalarSh2NamedGrid() {
}


bool ScalarSh2NamedGrid::sameAs(const Action& other) const {
    const ScalarSh2NamedGrid* o = dynamic_cast<const ScalarSh2NamedGrid*>(&other);
    return o && (gridname_ == o->gridname_);
}


void ScalarSh2NamedGrid::print(std::ostream& out) const {
    out << "ScalarSh2NamedGrid[gridname=" << gridname_ << "]";
}


const repres::Representation* ScalarSh2NamedGrid::outputRepresentation() const {
    const namedgrids::NamedGrid& ng = namedgrids::NamedGrid::lookup(gridname_);
    return ng.representation();
}


namespace {
static ActionBuilder< ScalarSh2NamedGrid > __action("transform.scalar-sh2namedgrid");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

