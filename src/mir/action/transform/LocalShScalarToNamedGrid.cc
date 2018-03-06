/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/transform/LocalShScalarToNamedGrid.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/namedgrids/NamedGrid.h"

namespace mir {
namespace action {
namespace transform {


LocalShScalarToNamedGrid::LocalShScalarToNamedGrid(const param::MIRParametrisation& parametrisation):
    LocalShScalarToGridded(parametrisation) {

    ASSERT(parametrisation_.userParametrisation().get("gridname", gridname_));

}


LocalShScalarToNamedGrid::~LocalShScalarToNamedGrid() {
}


bool LocalShScalarToNamedGrid::sameAs(const Action& other) const {
    const LocalShScalarToNamedGrid* o = dynamic_cast<const LocalShScalarToNamedGrid*>(&other);
    return o && (gridname_ == o->gridname_);
}


void LocalShScalarToNamedGrid::print(std::ostream& out) const {
    out << "LocalShScalarToNamedGrid[gridname=" << gridname_ << "]";
}


const char* LocalShScalarToNamedGrid::name() const {
    return "LocalShScalarToNamedGrid";
}


const repres::Representation* LocalShScalarToNamedGrid::outputRepresentation() const {
    const namedgrids::NamedGrid& ng = namedgrids::NamedGrid::lookup(gridname_);
    return ng.representation();
}


void LocalShScalarToNamedGrid::setTransOptions(atlas::util::Config& options) const {
    options.set(atlas::option::type("local"));
}


namespace {
static ActionBuilder< LocalShScalarToNamedGrid > __action("transform.local-sh-scalar-to-namedgrid");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

