/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/transform/LocalShVodTouvNamedGrid.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/namedgrids/NamedGrid.h"

namespace mir {
namespace action {
namespace transform {


LocalShVodTouvNamedGrid::LocalShVodTouvNamedGrid(const param::MIRParametrisation& parametrisation):
    LocalShVodTouvGridded(parametrisation) {

    ASSERT(parametrisation_.userParametrisation().get("gridname", gridname_));

}


LocalShVodTouvNamedGrid::~LocalShVodTouvNamedGrid() {
}


bool LocalShVodTouvNamedGrid::sameAs(const Action& other) const {
    const LocalShVodTouvNamedGrid* o = dynamic_cast<const LocalShVodTouvNamedGrid*>(&other);
    return o && (gridname_ == o->gridname_);
}


void LocalShVodTouvNamedGrid::print(std::ostream& out) const {
    out << "LocalShVodTouvNamedGrid[gridname=" << gridname_ << "]";
}


const char* LocalShVodTouvNamedGrid::name() const {
    return "LocalShVodTouvNamedGrid";
}


const repres::Representation* LocalShVodTouvNamedGrid::outputRepresentation() const {
    const namedgrids::NamedGrid& ng = namedgrids::NamedGrid::lookup(gridname_);
    return ng.representation();
}


void LocalShVodTouvNamedGrid::setTransOptions(atlas::util::Config& options) const {
    options.set(atlas::option::type("local"));
}


namespace {
static ActionBuilder< LocalShVodTouvNamedGrid > __action("transform.local-sh-vod-to-uv-namedgrid");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

