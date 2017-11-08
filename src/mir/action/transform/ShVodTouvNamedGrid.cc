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


#include "mir/action/transform/ShVodTouvNamedGrid.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/namedgrids/NamedGrid.h"

namespace mir {
namespace action {
namespace transform {


ShVodTouvNamedGrid::ShVodTouvNamedGrid(const param::MIRParametrisation& parametrisation):
    ShVodTouvGridded(parametrisation) {

    ASSERT(parametrisation_.userParametrisation().get("gridname", gridname_));

}


ShVodTouvNamedGrid::~ShVodTouvNamedGrid() {
}


bool ShVodTouvNamedGrid::sameAs(const Action& other) const {
    const ShVodTouvNamedGrid* o = dynamic_cast<const ShVodTouvNamedGrid*>(&other);
    return o && (gridname_ == o->gridname_);
}


void ShVodTouvNamedGrid::print(std::ostream& out) const {
    out << "ShVodTouvNamedGrid[gridname=" << gridname_ << "]";
}


const repres::Representation* ShVodTouvNamedGrid::outputRepresentation() const {
    const namedgrids::NamedGrid& ng = namedgrids::NamedGrid::lookup(gridname_);
    return ng.representation();
}


namespace {
static ActionBuilder< ShVodTouvNamedGrid > __action("transform.sh-vod-to-uv-namedgrid");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

