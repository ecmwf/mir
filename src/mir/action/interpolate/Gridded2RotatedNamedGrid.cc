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


#include "mir/action/interpolate/Gridded2RotatedNamedGrid.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"

#include "mir/key/grid/NamedGrid.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace action {
namespace interpolate {


Gridded2RotatedNamedGrid::Gridded2RotatedNamedGrid(const param::MIRParametrisation& parametrisation) :
    Gridded2RotatedGrid(parametrisation) {

    ASSERT(parametrisation_.userParametrisation().get("gridname", gridname_));
}


Gridded2RotatedNamedGrid::~Gridded2RotatedNamedGrid() = default;


bool Gridded2RotatedNamedGrid::sameAs(const Action& other) const {
    auto o = dynamic_cast<const Gridded2RotatedNamedGrid*>(&other);
    return (o != nullptr) && (gridname_ == o->gridname_) && Gridded2RotatedGrid::sameAs(other);
}


void Gridded2RotatedNamedGrid::print(std::ostream& out) const {
    out << "Gridded2RotatedNamedGrid["
           "gridname="
        << gridname_
        << ","
           "rotation="
        << rotation() << ",";
    Gridded2RotatedGrid::print(out);
    out << "]";
}


const repres::Representation* Gridded2RotatedNamedGrid::outputRepresentation() const {
    const auto& ng = key::grid::NamedGrid::lookup(gridname_);
    return ng.representation(rotation());
}


const char* Gridded2RotatedNamedGrid::name() const {
    return "Gridded2RotatedNamedGrid";
}


static ActionBuilder<Gridded2RotatedNamedGrid> grid2grid("interpolate.grid2rotated-namedgrid");


}  // namespace interpolate
}  // namespace action
}  // namespace mir
