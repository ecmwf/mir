// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/interpolate/Gridded2OctahedralGG.h"

#include <ostream>

#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/ReducedOctahedral.h"
#include "mir/util/Exceptions.h"


namespace mir::action::interpolate {


Gridded2OctahedralGG::Gridded2OctahedralGG(const param::MIRParametrisation& param) : Gridded2UnrotatedGrid(param) {
    ASSERT(parametrisation().userParametrisation().get("octahedral", N_));
}


bool Gridded2OctahedralGG::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const Gridded2OctahedralGG*>(&other);
    return (o != nullptr) && (N_ == o->N_) && Gridded2GriddedInterpolation::sameAs(other);
}


void Gridded2OctahedralGG::print(std::ostream& out) const {
    out << "Gridded2OctahedralGG["
           "N="
        << N_ << ",";
    Gridded2UnrotatedGrid::print(out);
    out << "]";
}


const repres::Representation* Gridded2OctahedralGG::outputRepresentation() const {
    return new repres::gauss::reduced::ReducedOctahedral(N_);
}


bool Gridded2OctahedralGG::getGriddedTargetName(std::string& name) const {
    name = "O" + std::to_string(N_);
    return true;
}


const char* Gridded2OctahedralGG::name() const {
    return "Gridded2OctahedralGG";
}


static const ActionBuilder<Gridded2OctahedralGG> grid2grid("interpolate.grid2octahedral-gg");


}  // namespace mir::action::interpolate
