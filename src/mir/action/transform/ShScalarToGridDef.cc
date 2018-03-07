/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/transform/ShScalarToGridDef.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/other/UnstructuredGrid.h"


namespace mir {
namespace action {
namespace transform {


ShScalarToGridDef::ShScalarToGridDef(const param::MIRParametrisation& parametrisation):
    ShScalarToGridded(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("griddef", griddef_));
}


ShScalarToGridDef::~ShScalarToGridDef() {
}


bool ShScalarToGridDef::sameAs(const Action& other) const {
    const ShScalarToGridDef* o = dynamic_cast<const ShScalarToGridDef*>(&other);
    return o && (griddef_ == o->griddef_);
}


void ShScalarToGridDef::print(std::ostream &out) const {
    out << "ShScalarToGridDef[";
    ShToGridded::print(out);
    out << ",griddef=.../" << eckit::PathName(griddef_).baseName()
        << "]";
}


const char* ShScalarToGridDef::name() const {
    return "ShScalarToGridDef";
}


const repres::Representation *ShScalarToGridDef::outputRepresentation() const {
    return new repres::other::UnstructuredGrid(griddef_);
}


namespace {
static ActionBuilder< ShScalarToGridDef > __action("transform.sh-scalar-to-griddef");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

