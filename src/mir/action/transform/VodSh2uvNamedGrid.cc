/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Feb 2017


#include "mir/action/transform/VodSh2uvNamedGrid.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/namedgrids/NamedGrid.h"

namespace mir {
namespace action {
namespace transform {


VodSh2uvNamedGrid::VodSh2uvNamedGrid(const param::MIRParametrisation& parametrisation):
    VodSh2uvGridded(parametrisation) {

    ASSERT(parametrisation_.get("user.gridname", gridname_));

}


VodSh2uvNamedGrid::~VodSh2uvNamedGrid() {
}


bool VodSh2uvNamedGrid::sameAs(const Action& other) const {
    const VodSh2uvNamedGrid* o = dynamic_cast<const VodSh2uvNamedGrid*>(&other);
    return o && (gridname_ == o->gridname_);
}


void VodSh2uvNamedGrid::print(std::ostream& out) const {
    out << "VodSh2uvNamedGrid[gridname=" << gridname_ << "]";
}


const repres::Representation* VodSh2uvNamedGrid::outputRepresentation() const {
    const namedgrids::NamedGrid& ng = namedgrids::NamedGrid::lookup(gridname_);
    return ng.representation();
}


namespace {
static ActionBuilder< VodSh2uvNamedGrid > __action("transform.vod-sh-to-uv-namedgrid");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

