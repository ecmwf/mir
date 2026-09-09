// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/interpolate/Gridded2NamedGrid.h"

#include <ostream>

#include "mir/key/grid/Grid.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::action::interpolate {


Gridded2NamedGrid::Gridded2NamedGrid(const param::MIRParametrisation& param) : Gridded2UnrotatedGrid(param) {
    ASSERT(key::grid::Grid::get("grid", grid_, parametrisation()));
    ASSERT(!grid_.empty());
}


bool Gridded2NamedGrid::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const Gridded2NamedGrid*>(&other);
    return (o != nullptr) && (grid_ == o->grid_) && Gridded2GriddedInterpolation::sameAs(other);
}


void Gridded2NamedGrid::print(std::ostream& out) const {
    out << "Gridded2NamedGrid[grid=" << grid_ << ",";
    Gridded2UnrotatedGrid::print(out);
    out << "]";
}


const repres::Representation* Gridded2NamedGrid::outputRepresentation() const {
    const auto& ng = key::grid::Grid::lookup(grid_);
    return ng.representation();
}


bool Gridded2NamedGrid::getGriddedTargetName(std::string& name) const {
    name = grid_;
    return true;
}


const char* Gridded2NamedGrid::name() const {
    return "Gridded2NamedGrid";
}


static const ActionBuilder<Gridded2NamedGrid> grid2grid("interpolate.grid2namedgrid");


}  // namespace mir::action::interpolate
