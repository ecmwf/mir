/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @date May 2017


#include "mir/style/IntermediateGridNone.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"


namespace mir {
namespace style {


namespace {
static IntermediateGridBuilder<IntermediateGridNone> __intermediate_grid_none("none");
}


IntermediateGridNone::IntermediateGridNone(const param::MIRParametrisation& parametrisation) :
    IntermediateGrid(parametrisation) {
}


void IntermediateGridNone::print(std::ostream& out) const {
    out << "IntermediateGridNone[]";
}


bool IntermediateGridNone::active() const {
    return false;
}


std::string IntermediateGridNone::getGridname() const {
    std::ostringstream os;
    os << "IntermediateGridNone::getGridname() not implemented for " << *this;
    throw eckit::SeriousBug(os.str());
}


}  // namespace style
}  // namespace mir

