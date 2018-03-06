/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/transform/LocalShScalarToGridDef.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/other/UnstructuredGrid.h"


namespace mir {
namespace action {
namespace transform {


LocalShScalarToGridDef::LocalShScalarToGridDef(const param::MIRParametrisation& parametrisation):
    LocalShScalarToGridded(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("griddef", griddef_));
}


LocalShScalarToGridDef::~LocalShScalarToGridDef() {
}


bool LocalShScalarToGridDef::sameAs(const Action& other) const {
    const LocalShScalarToGridDef* o = dynamic_cast<const LocalShScalarToGridDef*>(&other);
    return o && (griddef_ == o->griddef_);
}


void LocalShScalarToGridDef::print(std::ostream &out) const {
    out << "LocalShScalarToGridDef["
        << "griddef=.../" << eckit::PathName(griddef_).baseName()
        << "]";
}


const char* LocalShScalarToGridDef::name() const {
    return "LocalShScalarToGridDef";
}


const repres::Representation *LocalShScalarToGridDef::outputRepresentation() const {
    return new repres::other::UnstructuredGrid(griddef_);
}


void LocalShScalarToGridDef::setTransOptions(atlas::util::Config& options) const {
    options.set(atlas::option::type("local"));
}


namespace {
static ActionBuilder< LocalShScalarToGridDef > __action("transform.local-sh-scalar-to-griddef");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

