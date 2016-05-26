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

#include "mir/action/transform/Sh2Octahedral.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/ReducedOctahedral.h"


namespace mir {
namespace action {


Sh2Octahedral::Sh2Octahedral(const param::MIRParametrisation& parametrisation):
    Sh2GriddedTransform(parametrisation) {

    ASSERT(parametrisation_.get("user.octahedral", N_));

}

bool Sh2Octahedral::sameAs(const Action& other) const {
    const Sh2Octahedral* o = dynamic_cast<const Sh2Octahedral*>(&other);
    return o && (N_ == o->N_);
}

Sh2Octahedral::~Sh2Octahedral() {
}


void Sh2Octahedral::print(std::ostream& out) const {
    out << "Sh2Octahedral[N=" << N_ << "]";
}


const repres::Representation* Sh2Octahedral::outputRepresentation() const {
    return new repres::reduced::ReducedOctahedral(N_);
}


namespace {
static ActionBuilder< Sh2Octahedral > grid2grid("transform.sh2octahedral-gg");
}


}  // namespace action
}  // namespace mir

