// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/interpolate/Gridded2RotatedRegularGG.h"

#include <ostream>

#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/regular/RotatedGG.h"
#include "mir/util/Exceptions.h"


namespace mir::action::interpolate {


Gridded2RotatedRegularGG::Gridded2RotatedRegularGG(const param::MIRParametrisation& param) :
    Gridded2RotatedGrid(param), N_(0) {
    ASSERT(parametrisation().userParametrisation().get("regular", N_));
    ASSERT(N_ > 0);
}


bool Gridded2RotatedRegularGG::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const Gridded2RotatedRegularGG*>(&other);
    return (o != nullptr) && (N_ == o->N_) && Gridded2RotatedGrid::sameAs(other);
}


void Gridded2RotatedRegularGG::print(std::ostream& out) const {
    out << "Gridded2RotatedRegularGG["
           "N="
        << N_
        << ","
           "rotation="
        << rotation() << ",";
    Gridded2RotatedGrid::print(out);
    out << "]";
}


const repres::Representation* Gridded2RotatedRegularGG::outputRepresentation() const {
    return new repres::gauss::regular::RotatedGG(N_, rotation());
}


const char* Gridded2RotatedRegularGG::name() const {
    return "Gridded2RotatedRegularGG";
}


static const ActionBuilder<Gridded2RotatedRegularGG> grid2grid("interpolate.grid2rotated-regular-gg");


}  // namespace mir::action::interpolate
