/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/transform/ShVodTouvPoints.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/other/UnstructuredGrid.h"


namespace mir {
namespace action {
namespace transform {


ShVodTouvPoints::ShVodTouvPoints(const param::MIRParametrisation &parametrisation):
    ShVodTouvGridded(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("latitudes", latitudes_));
    ASSERT(parametrisation_.userParametrisation().get("longitudes", longitudes_));

    ASSERT(latitudes_.size() == longitudes_.size());
    local(true);
}


ShVodTouvPoints::~ShVodTouvPoints() {
}


bool ShVodTouvPoints::sameAs(const Action& other) const {
    const ShVodTouvPoints* o = dynamic_cast<const ShVodTouvPoints*>(&other);
    return o && (latitudes_ == o->latitudes_) && (longitudes_ == o->longitudes_);
}


void ShVodTouvPoints::print(std::ostream &out) const {
    out << "ShVodTouvPoints[";
    ShToGridded::print(out);
    out << ",points=" << latitudes_.size()
        << "]";
}

const char* ShVodTouvPoints::name() const {
    return "ShVodTouvPoints";
}

const repres::Representation *ShVodTouvPoints::outputRepresentation() const {
    return new repres::other::UnstructuredGrid(latitudes_, longitudes_);
}


namespace {
static ActionBuilder< ShVodTouvPoints > __action("transform.sh-vod-to-uv-points");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

