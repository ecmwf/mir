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

#include "mir/action/interpolate/Gridded2RegularGG.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "mir/repres/gauss/regular/RegularGG.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace action {


Gridded2RegularGG::Gridded2RegularGG(const param::MIRParametrisation& parametrisation):
    Gridded2GriddedInterpolation(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("regular", N_));
}


Gridded2RegularGG::~Gridded2RegularGG() {
}


bool Gridded2RegularGG::sameAs(const Action& other) const {
    const Gridded2RegularGG* o = dynamic_cast<const Gridded2RegularGG*>(&other);
    return o && (N_ == o->N_) && Gridded2GriddedInterpolation::sameAs(other);
}

void Gridded2RegularGG::print(std::ostream& out) const {
    out << "Gridded2RegularGG[N=" << N_ << "]";
}


const repres::Representation* Gridded2RegularGG::outputRepresentation() const {
    return new repres::gauss::regular::RegularGG(N_);
}


namespace {
static ActionBuilder< Gridded2RegularGG > grid2grid("interpolate.grid2regular-gg");
}


}  // namespace action
}  // namespace mir

