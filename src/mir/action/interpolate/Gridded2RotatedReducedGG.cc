// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/interpolate/Gridded2RotatedReducedGG.h"

#include <ostream>

#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/RotatedClassic.h"
#include "mir/util/Exceptions.h"


namespace mir::action::interpolate {


Gridded2RotatedReducedGG::Gridded2RotatedReducedGG(const param::MIRParametrisation& param) :
    Gridded2RotatedGrid(param), N_(0) {
    ASSERT(parametrisation().userParametrisation().get("reduced", N_));
    ASSERT(N_ > 0);
}


bool Gridded2RotatedReducedGG::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const Gridded2RotatedReducedGG*>(&other);
    return (o != nullptr) && (N_ == o->N_) && Gridded2RotatedGrid::sameAs(other);
}


void Gridded2RotatedReducedGG::print(std::ostream& out) const {
    out << "Gridded2RotatedReducedGG["
           "N="
        << N_
        << ","
           "rotation="
        << rotation() << ",";
    Gridded2RotatedGrid::print(out);
    out << "]";
}


const repres::Representation* Gridded2RotatedReducedGG::outputRepresentation() const {
    return new repres::gauss::reduced::RotatedClassic(N_, rotation());
}


const char* Gridded2RotatedReducedGG::name() const {
    return "Gridded2RotatedReducedGG";
}


static const ActionBuilder<Gridded2RotatedReducedGG> grid2grid("interpolate.grid2rotated-reduced-gg");


}  // namespace mir::action::interpolate
