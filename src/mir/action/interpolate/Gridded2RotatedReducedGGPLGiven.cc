// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/interpolate/Gridded2RotatedReducedGGPLGiven.h"

#include <ostream>
#include <vector>

#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/RotatedFromPL.h"
#include "mir/util/Exceptions.h"


namespace mir::action::interpolate {


Gridded2RotatedReducedGGPLGiven::Gridded2RotatedReducedGGPLGiven(const param::MIRParametrisation& param) :
    Gridded2RotatedGrid(param) {
    ASSERT(parametrisation().userParametrisation().get("pl", pl_));
    ASSERT(!pl_.empty());
}


bool Gridded2RotatedReducedGGPLGiven::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const Gridded2RotatedReducedGGPLGiven*>(&other);
    return (o != nullptr) && (pl_ == o->pl_) && Gridded2RotatedGrid::sameAs(other);
}


void Gridded2RotatedReducedGGPLGiven::print(std::ostream& out) const {
    out << "Gridded2RotatedReducedGGPLGiven["
           "pl="
        << pl_.size()
        << ","
           "rotation="
        << rotation() << ",";
    Gridded2RotatedGrid::print(out);
    out << "]";
}


const repres::Representation* Gridded2RotatedReducedGGPLGiven::outputRepresentation() const {
    size_t N = pl_.size() / 2;
    return new repres::gauss::reduced::RotatedFromPL(N, pl_, rotation());
}


const char* Gridded2RotatedReducedGGPLGiven::name() const {
    return "Gridded2RotatedReducedGGPLGiven";
}


static const ActionBuilder<Gridded2RotatedReducedGGPLGiven> grid2grid("interpolate.grid2rotated-reduced-gg-pl-given");


}  // namespace mir::action::interpolate
