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

#include "mir/action/interpolate/Gridded2ReducedGG.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "mir/repres/gauss/reduced/ReducedClassic.h"
#include "mir/param/MIRParametrisation.h"

namespace mir {
namespace action {


Gridded2ReducedGG::Gridded2ReducedGG(const param::MIRParametrisation& parametrisation):
    Gridded2GriddedInterpolation(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("reduced", N_));
}


Gridded2ReducedGG::~Gridded2ReducedGG() {
}

bool Gridded2ReducedGG::sameAs(const Action& other) const {
    const Gridded2ReducedGG* o = dynamic_cast<const Gridded2ReducedGG*>(&other);
    return o && (N_ == o->N_) && Gridded2GriddedInterpolation::sameAs(other);
}

void Gridded2ReducedGG::print(std::ostream& out) const {
    out << "Gridded2ReducedGG[N=" << N_ << ",";
    Gridded2GriddedInterpolation::print(out);
    out << "]";
}


const repres::Representation* Gridded2ReducedGG::outputRepresentation() const {
    return new repres::gauss::reduced::ReducedClassic(N_);
}


namespace {
static ActionBuilder< Gridded2ReducedGG > grid2grid("interpolate.grid2reduced-gg");
}


}  // namespace action
}  // namespace mir

