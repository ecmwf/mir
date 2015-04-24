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

#include "soyuz/action/Gridded2RegularGG.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "eckit/utils/Translator.h"
#include "soyuz/param/MIRParametrisation.h"

#include "soyuz/repres/RegularGG.h"


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
    eckit::Translator<std::string, int> s2i;
    std::string value;

    ASSERT(parametrisation_.get("regular", value));

    return new repres::RegularGG(s2i(value));
}


namespace {
static ActionBuilder< Gridded2RegularGG > grid2grid("interpolate.grid2regular-gg");
}


}  // namespace action
}  // namespace mir

