// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/action/interpolate/Gridded2ReducedLL.h"

#include <ostream>

#include "mir/util/Exceptions.h"


namespace mir::action::interpolate {


Gridded2ReducedLL::Gridded2ReducedLL(const param::MIRParametrisation& param) : Gridded2UnrotatedGrid(param) {}


bool Gridded2ReducedLL::sameAs(const Action& /*other*/) const {
    NOTIMP;
}


void Gridded2ReducedLL::print(std::ostream& out) const {
    out << "Gridded2ReducedLL[";
    Gridded2UnrotatedGrid::print(out);
    out << "]";
}


const repres::Representation* Gridded2ReducedLL::outputRepresentation() const {
    NOTIMP;
}


const char* Gridded2ReducedLL::name() const {
    return "Gridded2ReducedLL";
}


static const ActionBuilder<Gridded2ReducedLL> grid2grid("interpolate.grid2reduced-ll");


}  // namespace mir::action::interpolate
