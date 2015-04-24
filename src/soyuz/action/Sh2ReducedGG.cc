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

#include "soyuz/action/Sh2ReducedGG.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"


namespace mir {
namespace action {


Sh2ReducedGG::Sh2ReducedGG(const param::MIRParametrisation& parametrisation):
    Sh2GriddedTransform(parametrisation) {
}


Sh2ReducedGG::~Sh2ReducedGG() {
}


void Sh2ReducedGG::print(std::ostream& out) const {
    out << "Sh2ReducedGG[]";
}


repres::Representation* Sh2ReducedGG::outputRepresentation(const repres::Representation* inputRepres) const {
    return 0;
}


namespace {
static ActionBuilder< Sh2ReducedGG > grid2grid("transform.sh2reduced-gg");
}


}  // namespace action
}  // namespace mir

