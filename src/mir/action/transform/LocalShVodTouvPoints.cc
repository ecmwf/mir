/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/transform/LocalShVodTouvPoints.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/other/UnstructuredGrid.h"


namespace mir {
namespace action {
namespace transform {


LocalShVodTouvPoints::LocalShVodTouvPoints(const param::MIRParametrisation &parametrisation):
    LocalShVodTouvGridded(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("latitudes", latitudes_));
    ASSERT(parametrisation_.userParametrisation().get("longitudes", longitudes_));

    ASSERT(latitudes_.size() == longitudes_.size());
}


LocalShVodTouvPoints::~LocalShVodTouvPoints() {
}


bool LocalShVodTouvPoints::sameAs(const Action& other) const {
    const LocalShVodTouvPoints* o = dynamic_cast<const LocalShVodTouvPoints*>(&other);
    return o && (latitudes_ == o->latitudes_) && (longitudes_ == o->longitudes_);
}


void LocalShVodTouvPoints::print(std::ostream &out) const {
    out << "LocalShVodTouvPoints[points=" << latitudes_.size() << "]";
}

const char* LocalShVodTouvPoints::name() const {
    return "LocalShVodTouvPoints";
}

const repres::Representation *LocalShVodTouvPoints::outputRepresentation() const {
    return new repres::other::UnstructuredGrid(latitudes_, longitudes_);
}


void LocalShVodTouvPoints::setTransOptions(atlas::util::Config& options) const {
    options.set(atlas::option::type("local"));
}


namespace {
static ActionBuilder< LocalShVodTouvPoints > __action("transform.local-sh-vod-to-uv-points");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

