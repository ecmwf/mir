/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/action/interpolate/Gridded2ReducedLL.h"

#include <ostream>

#include "mir/util/Exceptions.h"


namespace mir {
namespace action {
namespace interpolate {


Gridded2ReducedLL::Gridded2ReducedLL(const param::MIRParametrisation& parametrisation) :
    Gridded2UnrotatedGrid(parametrisation) {}


Gridded2ReducedLL::~Gridded2ReducedLL() = default;


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


static ActionBuilder<Gridded2ReducedLL> grid2grid("interpolate.grid2reduced-ll");


}  // namespace interpolate
}  // namespace action
}  // namespace mir
