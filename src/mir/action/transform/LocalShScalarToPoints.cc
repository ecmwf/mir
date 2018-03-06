/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/transform/LocalShScalarToPoints.h"

#include <iostream>
#include <vector>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/other/UnstructuredGrid.h"


namespace mir {
namespace action {
namespace transform {


LocalShScalarToPoints::LocalShScalarToPoints(const param::MIRParametrisation& parametrisation):
    LocalShScalarToGridded(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("latitudes", latitudes_));
    ASSERT(parametrisation_.userParametrisation().get("longitudes", longitudes_));

    ASSERT(latitudes_.size() == longitudes_.size());
}


LocalShScalarToPoints::~LocalShScalarToPoints() {
}


bool LocalShScalarToPoints::sameAs(const Action& other) const {
    const LocalShScalarToPoints* o = dynamic_cast<const LocalShScalarToPoints*>(&other);
    return o && (latitudes_ == o->latitudes_) && (longitudes_ == o->longitudes_);
}


void LocalShScalarToPoints::print(std::ostream &out) const {
    out << "LocalShScalarToPoints[points=" << latitudes_.size() << "]";
}


const char* LocalShScalarToPoints::name() const {
    return "LocalShScalarToPoints";
}


const repres::Representation *LocalShScalarToPoints::outputRepresentation() const {
    return new repres::other::UnstructuredGrid(latitudes_, longitudes_);
}


void LocalShScalarToPoints::setTransOptions(atlas::util::Config& options) const {
    options.set(atlas::option::type("local"));
}


namespace {
static ActionBuilder< LocalShScalarToPoints > __action("transform.local-sh-scalar-to-points");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

