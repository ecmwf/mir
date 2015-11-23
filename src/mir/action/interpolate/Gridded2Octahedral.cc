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

#include "mir/action/interpolate/Gridded2Octahedral.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "mir/repres/gauss/reduced/ReducedOctahedral.h"
#include "mir/param/MIRParametrisation.h"

namespace mir {
namespace action {


Gridded2Octahedral::Gridded2Octahedral(const param::MIRParametrisation& parametrisation):
    Gridded2GriddedInterpolation(parametrisation) {
    ASSERT(parametrisation_.get("user.octahedral", N_));
}


Gridded2Octahedral::~Gridded2Octahedral() {
}


void Gridded2Octahedral::print(std::ostream& out) const {
    out << "Gridded2Octahedral[N=" << N_ << "]";
}


const repres::Representation* Gridded2Octahedral::outputRepresentation() const {
    return new repres::reduced::ReducedOctahedral(N_);
}


namespace {
static ActionBuilder< Gridded2Octahedral > grid2grid("interpolate.grid2octahedral-gg");
}


}  // namespace action
}  // namespace mir

