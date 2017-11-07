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

#include "mir/action/interpolate/Gridded2ReducedGGPLGiven.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "mir/repres/gauss/reduced/ReducedFromPL.h"
#include "mir/param/MIRParametrisation.h"

namespace mir {
namespace action {


Gridded2ReducedGGPLGiven::Gridded2ReducedGGPLGiven(const param::MIRParametrisation& parametrisation):
    Gridded2GriddedInterpolation(parametrisation) {
    ASSERT(parametrisation_.user().get("pl", pl_));
}


Gridded2ReducedGGPLGiven::~Gridded2ReducedGGPLGiven() {
}


bool Gridded2ReducedGGPLGiven::sameAs(const Action& other) const {
    const Gridded2ReducedGGPLGiven* o = dynamic_cast<const Gridded2ReducedGGPLGiven*>(&other);
    return o && (pl_ == o->pl_);
}

void Gridded2ReducedGGPLGiven::print(std::ostream& out) const {
    out << "Gridded2ReducedGGPLGiven[pl=" << pl_.size() << "]";
}


const repres::Representation* Gridded2ReducedGGPLGiven::outputRepresentation() const {
    return new repres::gauss::reduced::ReducedFromPL(pl_);
}


namespace {
static ActionBuilder< Gridded2ReducedGGPLGiven > grid2grid("interpolate.grid2reduced-gg-pl-given");
}


}  // namespace action
}  // namespace mir

