// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/interpolate/Gridded2ReducedLLPLGiven.h"

#include <ostream>

#include "mir/util/Exceptions.h"


namespace mir::action::interpolate {


Gridded2ReducedLLPLGiven::Gridded2ReducedLLPLGiven(const param::MIRParametrisation& param) :
    Gridded2UnrotatedGrid(param) {}


bool Gridded2ReducedLLPLGiven::sameAs(const Action& /*other*/) const {
    NOTIMP;
}


void Gridded2ReducedLLPLGiven::print(std::ostream& out) const {
    out << "Gridded2ReducedLLPLGiven[";
    Gridded2UnrotatedGrid::print(out);
    out << "]";
}


const repres::Representation* Gridded2ReducedLLPLGiven::outputRepresentation() const {
    NOTIMP;
}


const char* Gridded2ReducedLLPLGiven::name() const {
    return "Gridded2ReducedLLPLGiven";
}


static const ActionBuilder<Gridded2ReducedLLPLGiven> grid2grid("interpolate.grid2reduced-ll-pl-given");


}  // namespace mir::action::interpolate
