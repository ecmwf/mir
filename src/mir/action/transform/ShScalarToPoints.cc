/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/transform/ShScalarToPoints.h"

#include <iostream>
#include <vector>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/other/UnstructuredGrid.h"


namespace mir {
namespace action {
namespace transform {


ShScalarToPoints::ShScalarToPoints(const param::MIRParametrisation& parametrisation):
    ShScalarToGridded(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("latitudes", latitudes_));
    ASSERT(parametrisation_.userParametrisation().get("longitudes", longitudes_));

    ASSERT(latitudes_.size() == longitudes_.size());
}


ShScalarToPoints::~ShScalarToPoints() {
}


bool ShScalarToPoints::sameAs(const Action& other) const {
    const ShScalarToPoints* o = dynamic_cast<const ShScalarToPoints*>(&other);
    return o && (latitudes_ == o->latitudes_) && (longitudes_ == o->longitudes_);
}


void ShScalarToPoints::print(std::ostream &out) const {
    out << "ShScalarToPoints[";
    ShToGridded::print(out);
    out << ",points=" << latitudes_.size()
        << "]";
}


const char* ShScalarToPoints::name() const {
    return "ShScalarToPoints";
}


const repres::Representation *ShScalarToPoints::outputRepresentation() const {
    return new repres::other::UnstructuredGrid(latitudes_, longitudes_);
}


namespace {
static ActionBuilder< ShScalarToPoints > __action("transform.sh-scalar-to-points");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

