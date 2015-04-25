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

#include "soyuz/action/Gridded2RegularLL.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"


#include "soyuz/repres/RegularLL.h"
#include "soyuz/param/MIRParametrisation.h"


namespace mir {
namespace action {


Gridded2RegularLL::Gridded2RegularLL(const param::MIRParametrisation &parametrisation):
    Gridded2GriddedInterpolation(parametrisation) {
}


Gridded2RegularLL::~Gridded2RegularLL() {
}


void Gridded2RegularLL::print(std::ostream &out) const {
    out << "Gridded2RegularLL[]";
}


repres::Representation *Gridded2RegularLL::outputRepresentation(const repres::Representation *inputRepres) const {
    std::vector<double> value;

    ASSERT(parametrisation_.get("user.grid", value));

    double we = value[0];
    double ns = value[1];

    return new repres::RegularLL(util::BoundingBox(90, 0, -90, 360 - we), ns, we);
}


namespace {
static ActionBuilder< Gridded2RegularLL > grid2grid("interpolate.grid2regular-ll");
}


}  // namespace action
}  // namespace mir

