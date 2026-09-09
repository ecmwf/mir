// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/interpolate/Gridded2RegularGG.h"

#include <ostream>

#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/regular/RegularGG.h"
#include "mir/util/Exceptions.h"


namespace mir::action::interpolate {


Gridded2RegularGG::Gridded2RegularGG(const param::MIRParametrisation& param) : Gridded2UnrotatedGrid(param), N_(0) {
    ASSERT(parametrisation().userParametrisation().get("regular", N_));
    ASSERT(N_ > 0);
}


bool Gridded2RegularGG::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const Gridded2RegularGG*>(&other);
    return (o != nullptr) && (N_ == o->N_) && Gridded2GriddedInterpolation::sameAs(other);
}


void Gridded2RegularGG::print(std::ostream& out) const {
    out << "Gridded2RegularGG["
           "N="
        << N_ << ",";
    Gridded2UnrotatedGrid::print(out);
    out << "]";
}


const repres::Representation* Gridded2RegularGG::outputRepresentation() const {
    return new repres::gauss::regular::RegularGG(N_);
}


bool Gridded2RegularGG::getGriddedTargetName(std::string& name) const {
    name = "F" + std::to_string(N_);
    return true;
}


const char* Gridded2RegularGG::name() const {
    return "Gridded2RegularGG";
}


static const ActionBuilder<Gridded2RegularGG> grid2grid("interpolate.grid2regular-gg");


}  // namespace mir::action::interpolate
