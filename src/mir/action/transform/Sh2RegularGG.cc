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

#include "mir/action/transform/Sh2RegularGG.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/regular/RegularGG.h"


namespace mir {
namespace action {


Sh2RegularGG::Sh2RegularGG(const param::MIRParametrisation& parametrisation):
    Sh2GriddedTransform(parametrisation) {
    ASSERT(parametrisation_.get("user.regular", N_));
}


Sh2RegularGG::~Sh2RegularGG() {
}


void Sh2RegularGG::print(std::ostream& out) const {
    out << "Sh2RegularGG[N=" << N_ << "]";
}


const repres::Representation* Sh2RegularGG::outputRepresentation() const {
    return new repres::regular::RegularGG(N_);
}


namespace {
static ActionBuilder< Sh2RegularGG > grid2grid("transform.sh2regular-gg");
}


}  // namespace action
}  // namespace mir

