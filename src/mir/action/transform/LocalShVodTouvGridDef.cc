/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/transform/LocalShVodTouvGridDef.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/other/UnstructuredGrid.h"


namespace mir {
namespace action {
namespace transform {


LocalShVodTouvGridDef::LocalShVodTouvGridDef(const param::MIRParametrisation &parametrisation):
    LocalShVodTouvGridded(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("griddef", griddef_));
}


LocalShVodTouvGridDef::~LocalShVodTouvGridDef() {
}


bool LocalShVodTouvGridDef::sameAs(const Action& other) const {
    const LocalShVodTouvGridDef* o = dynamic_cast<const LocalShVodTouvGridDef*>(&other);
    return o && (griddef_ == o->griddef_);
}


void LocalShVodTouvGridDef::print(std::ostream &out) const {
    out << "LocalShVodTouvGridDef["
        << "griddef=.../" << eckit::PathName(griddef_).baseName()
        << "]";
}

const char* LocalShVodTouvGridDef::name() const {
    return "LocalShVodTouvGridDef";
}

const repres::Representation *LocalShVodTouvGridDef::outputRepresentation() const {
    return new repres::other::UnstructuredGrid(griddef_);
}


void LocalShVodTouvGridDef::setTransOptions(atlas::util::Config& options) const {
    options.set(atlas::option::type("local"));
}


namespace {
static ActionBuilder< LocalShVodTouvGridDef > __action("transform.local-sh-vod-to-uv-griddef");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

