/*
 * (C) Copyright 1996-2017 ECMWF.
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


#include "mir/action/transform/ShScalarToNamedGrid.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/namedgrids/NamedGrid.h"

namespace mir {
namespace action {
namespace transform {


ShScalarToNamedGrid::ShScalarToNamedGrid(const param::MIRParametrisation& parametrisation):
    ShScalarToGridded(parametrisation) {

    ASSERT(parametrisation_.user().get("gridname", gridname_));

}


ShScalarToNamedGrid::~ShScalarToNamedGrid() {
}


bool ShScalarToNamedGrid::sameAs(const Action& other) const {
    const ShScalarToNamedGrid* o = dynamic_cast<const ShScalarToNamedGrid*>(&other);
    return o && (gridname_ == o->gridname_);
}


void ShScalarToNamedGrid::print(std::ostream& out) const {
    out << "ShScalarToNamedGrid[gridname=" << gridname_ << "]";
}


const repres::Representation* ShScalarToNamedGrid::outputRepresentation() const {
    const namedgrids::NamedGrid& ng = namedgrids::NamedGrid::lookup(gridname_);
    return ng.representation();
}


namespace {
static ActionBuilder< ShScalarToNamedGrid > __action("transform.sh-scalar-to-namedgrid");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

