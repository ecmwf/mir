/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015

#include "mir/action/interpolate/Gridded2Points.h"

#include <iostream>

#include "eckit/filesystem/PathName.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/other/UnstructuredGrid.h"


namespace mir {
namespace action {


Gridded2Points::Gridded2Points(const param::MIRParametrisation& parametrisation):
    Gridded2GriddedInterpolation(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("latitudes", latitudes_));
    ASSERT(parametrisation_.userParametrisation().get("longitudes", longitudes_));

    ASSERT(latitudes_.size() == longitudes_.size());
}


Gridded2Points::~Gridded2Points() {
}

bool Gridded2Points::sameAs(const Action& other) const {
    const Gridded2Points* o = dynamic_cast<const Gridded2Points*>(&other);
    return o && (latitudes_ == o->latitudes_) && (longitudes_ == o->longitudes_) && Gridded2GriddedInterpolation::sameAs(other);
}

void Gridded2Points::print(std::ostream& out) const {
    out << "Gridded2Points[points=" << latitudes_.size() << ",";
    Gridded2GriddedInterpolation::print(out);
    out  << "]";
}


const repres::Representation* Gridded2Points::outputRepresentation() const {
    return new repres::other::UnstructuredGrid(latitudes_, longitudes_);
}

const char* Gridded2Points::name() const {
    return "Gridded2Points";
}


namespace {
static ActionBuilder< Gridded2Points > grid2grid("interpolate.grid2points");
}


}  // namespace action
}  // namespace mir

