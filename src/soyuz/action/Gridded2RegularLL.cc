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

#include "atlas/GridSpec.h"


namespace mir {
namespace action {


Gridded2RegularLL::Gridded2RegularLL(const param::MIRParametrisation& parametrisation):
    Gridded2GriddedInterpolation(parametrisation) {
}


Gridded2RegularLL::~Gridded2RegularLL() {
}


void Gridded2RegularLL::print(std::ostream& out) const {
    out << "Gridded2RegularLL[]";
}


atlas::GridSpec Gridded2RegularLL::outputGridSpec(const atlas::GridSpec& inputGridSpec) const {
   atlas::GridSpec result(inputGridSpec);
   return result;
}


namespace {
static ActionBuilder< Gridded2RegularLL > grid2grid("interpolate.grid2regular-ll");
}


}  // namespace action
}  // namespace mir

