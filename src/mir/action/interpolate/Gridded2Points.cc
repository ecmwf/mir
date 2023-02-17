/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/interpolate/Gridded2Points.h"

#include <ostream>

#include "mir/param/MIRParametrisation.h"
#include "mir/repres/other/UnstructuredGrid.h"
#include "mir/util/Exceptions.h"


namespace mir::action::interpolate {


Gridded2Points::Gridded2Points(const param::MIRParametrisation& parametrisation) :
    Gridded2UnrotatedGrid(parametrisation) {
    ASSERT(parametrisation_.get("latitudes", latitudes_));
    ASSERT(parametrisation_.get("longitudes", longitudes_));

    if (latitudes_.empty() || longitudes_.empty()) {
        throw exception::UserError("Gridded2Points: requires 'latitudes' and 'longitudes'");
    }
    ASSERT(latitudes_.size() == longitudes_.size());
}


Gridded2Points::~Gridded2Points() = default;


bool Gridded2Points::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const Gridded2Points*>(&other);
    return (o != nullptr) && (latitudes_ == o->latitudes_) && (longitudes_ == o->longitudes_) &&
           Gridded2GriddedInterpolation::sameAs(other);
}


void Gridded2Points::print(std::ostream& out) const {
    out << "Gridded2Points["
           "points="
        << latitudes_.size() << ",";
    Gridded2UnrotatedGrid::print(out);
    out << "]";
}


const repres::Representation* Gridded2Points::outputRepresentation() const {
    return new repres::other::UnstructuredGrid(latitudes_, longitudes_);
}


const char* Gridded2Points::name() const {
    return "Gridded2Points";
}


static const ActionBuilder<Gridded2Points> grid2grid("interpolate.grid2points");


}  // namespace mir::action::interpolate
