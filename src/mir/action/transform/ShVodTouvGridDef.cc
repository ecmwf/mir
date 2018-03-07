/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/transform/ShVodTouvGridDef.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/other/UnstructuredGrid.h"


namespace mir {
namespace action {
namespace transform {


ShVodTouvGridDef::ShVodTouvGridDef(const param::MIRParametrisation &parametrisation):
    ShVodTouvGridded(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("griddef", griddef_));
}


ShVodTouvGridDef::~ShVodTouvGridDef() {
}


bool ShVodTouvGridDef::sameAs(const Action& other) const {
    const ShVodTouvGridDef* o = dynamic_cast<const ShVodTouvGridDef*>(&other);
    return o && (griddef_ == o->griddef_);
}


void ShVodTouvGridDef::print(std::ostream &out) const {
    out << "ShVodTouvGridDef[";
    ShToGridded::print(out);
    out << "griddef=.../" << eckit::PathName(griddef_).baseName()
        << "]";
}

const char* ShVodTouvGridDef::name() const {
    return "ShVodTouvGridDef";
}

const repres::Representation *ShVodTouvGridDef::outputRepresentation() const {
    return new repres::other::UnstructuredGrid(griddef_);
}


namespace {
static ActionBuilder< ShVodTouvGridDef > __action("transform.sh-vod-to-uv-griddef");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

