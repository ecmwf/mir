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

#include <ostream>

#include "mir/key/grid/Grid.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::action::interpolate {


Gridded2RotatedNamedGrid::Gridded2RotatedNamedGrid(const param::MIRParametrisation& parametrisation) :
    Gridded2RotatedGrid(parametrisation) {
    ASSERT(key::grid::Grid::get("grid", grid_, parametrisation) && !grid_.empty());
}


bool Gridded2RotatedNamedGrid::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const Gridded2RotatedNamedGrid*>(&other);
    return (o != nullptr) && (grid_ == o->grid_) && Gridded2RotatedGrid::sameAs(other);
}


void Gridded2RotatedNamedGrid::print(std::ostream& out) const {
    out << "Gridded2RotatedNamedGrid[grid=" << grid_ << ",rotation=" << rotation() << ",";
    Gridded2RotatedGrid::print(out);
    out << "]";
}


const repres::Representation* Gridded2RotatedNamedGrid::outputRepresentation() const {
    const auto& ng = key::grid::Grid::lookup(grid_);
    return ng.representation(rotation());
}


const char* Gridded2RotatedNamedGrid::name() const {
    return "Gridded2RotatedNamedGrid";
}


static const ActionBuilder<Gridded2RotatedNamedGrid> grid2grid("interpolate.grid2rotated-namedgrid");


}  // namespace mir::action::interpolate
