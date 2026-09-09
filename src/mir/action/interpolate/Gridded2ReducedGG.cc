// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/interpolate/Gridded2ReducedGG.h"

#include <ostream>

#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/ReducedClassic.h"
#include "mir/util/Exceptions.h"


namespace mir::action::interpolate {


Gridded2ReducedGG::Gridded2ReducedGG(const param::MIRParametrisation& param) : Gridded2UnrotatedGrid(param) {
    ASSERT(parametrisation().userParametrisation().get("reduced", N_));
}


bool Gridded2ReducedGG::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const Gridded2ReducedGG*>(&other);
    return (o != nullptr) && (N_ == o->N_) && Gridded2GriddedInterpolation::sameAs(other);
}


void Gridded2ReducedGG::print(std::ostream& out) const {
    out << "Gridded2ReducedGG["
           "N="
        << N_ << ",";
    Gridded2UnrotatedGrid::print(out);
    out << "]";
}


const repres::Representation* Gridded2ReducedGG::outputRepresentation() const {
    return new repres::gauss::reduced::ReducedClassic(N_);
}


bool Gridded2ReducedGG::getGriddedTargetName(std::string& name) const {
    name = "N" + std::to_string(N_);
    return true;
}


const char* Gridded2ReducedGG::name() const {
    return "Gridded2ReducedGG";
}


static const ActionBuilder<Gridded2ReducedGG> grid2grid("interpolate.grid2reduced-gg");


}  // namespace mir::action::interpolate
