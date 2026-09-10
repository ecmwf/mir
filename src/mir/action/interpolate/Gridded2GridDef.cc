// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/interpolate/Gridded2GridDef.h"

#include <ostream>

#include "eckit/filesystem/PathName.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/repres/other/UnstructuredGrid.h"
#include "mir/util/Exceptions.h"


namespace mir::action::interpolate {


Gridded2GridDef::Gridded2GridDef(const param::MIRParametrisation& param) : Gridded2UnrotatedGrid(param) {
    ASSERT(parametrisation().userParametrisation().get("griddef", griddef_));
}


bool Gridded2GridDef::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const Gridded2GridDef*>(&other);
    return (o != nullptr) && (griddef_ == o->griddef_) && Gridded2GriddedInterpolation::sameAs(other);
}


void Gridded2GridDef::custom(std::ostream& out) const {
    out << "Gridded2GridDef["
           "griddef=.../"
        << eckit::PathName(griddef_).baseName() << "]";
}


void Gridded2GridDef::print(std::ostream& out) const {
    out << "Gridded2GridDef["
        << "griddef=" << griddef_ << ",";
    Gridded2UnrotatedGrid::print(out);
    out << "]";
}


const repres::Representation* Gridded2GridDef::outputRepresentation() const {
    return new repres::other::UnstructuredGrid(griddef_);
}


const char* Gridded2GridDef::name() const {
    return "Gridded2GridDef";
}


static const ActionBuilder<Gridded2GridDef> grid2grid("interpolate.grid2griddef");


}  // namespace mir::action::interpolate
