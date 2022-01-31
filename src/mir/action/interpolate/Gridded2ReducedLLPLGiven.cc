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


#include "mir/action/interpolate/Gridded2ReducedLLPLGiven.h"

#include <ostream>

#include "mir/util/Exceptions.h"


namespace mir {
namespace action {
namespace interpolate {


Gridded2ReducedLLPLGiven::Gridded2ReducedLLPLGiven(const param::MIRParametrisation& parametrisation) :
    Gridded2UnrotatedGrid(parametrisation) {}


Gridded2ReducedLLPLGiven::~Gridded2ReducedLLPLGiven() = default;


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


}  // namespace interpolate
}  // namespace action
}  // namespace mir
