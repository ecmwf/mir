/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015

#include "mir/action/interpolate/Gridded2ReducedLLPLGiven.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"


namespace mir {
namespace action {


Gridded2ReducedLLPLGiven::Gridded2ReducedLLPLGiven(const param::MIRParametrisation& parametrisation):
    Gridded2GriddedInterpolation(parametrisation) {
}


Gridded2ReducedLLPLGiven::~Gridded2ReducedLLPLGiven() {
}

bool Gridded2ReducedLLPLGiven::sameAs(const Action& other) const {
    NOTIMP;
}

void Gridded2ReducedLLPLGiven::print(std::ostream& out) const {
    out << "Gridded2ReducedLLPLGiven[]";
}


const repres::Representation* Gridded2ReducedLLPLGiven::outputRepresentation() const {
    NOTIMP;
}


namespace {
static ActionBuilder< Gridded2ReducedLLPLGiven > grid2grid("interpolate.grid2reduced-ll-pl-given");
}


}  // namespace action
}  // namespace mir

