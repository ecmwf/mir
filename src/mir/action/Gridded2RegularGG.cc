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

#include "mir/action/Gridded2RegularGG.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "eckit/utils/Translator.h"
#include "mir/param/MIRParametrisation.h"

#include "mir/repres/RegularGG.h"


namespace mir {
namespace action {


Gridded2RegularGG::Gridded2RegularGG(const param::MIRParametrisation& parametrisation):
    Gridded2GriddedInterpolation(parametrisation) {
}


Gridded2RegularGG::~Gridded2RegularGG() {
}


void Gridded2RegularGG::print(std::ostream& out) const {
    out << "Gridded2RegularGG[]";
}


repres::Representation* Gridded2RegularGG::outputRepresentation(const repres::Representation* inputRepres) const {
    long N;
    ASSERT(parametrisation_.get("regular", N));
    return new repres::RegularGG(N);
}


namespace {
static ActionBuilder< Gridded2RegularGG > grid2grid("interpolate.grid2regular-gg");
}


}  // namespace action
}  // namespace mir

