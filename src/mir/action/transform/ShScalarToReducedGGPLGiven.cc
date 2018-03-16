/*
 * (C) Copyright 1996- ECMWF.
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


#include "mir/action/transform/ShScalarToReducedGGPLGiven.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/ReducedFromPL.h"


namespace mir {
namespace action {
namespace transform {


ShScalarToReducedGGPLGiven::ShScalarToReducedGGPLGiven(const param::MIRParametrisation& parametrisation):
    ShScalarToGridded(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("pl", pl_));
}


ShScalarToReducedGGPLGiven::~ShScalarToReducedGGPLGiven() {
}


bool ShScalarToReducedGGPLGiven::sameAs(const Action& other) const {
    const ShScalarToReducedGGPLGiven* o = dynamic_cast<const ShScalarToReducedGGPLGiven*>(&other);
    return o && (pl_ == o->pl_);
}


void ShScalarToReducedGGPLGiven::print(std::ostream& out) const {
    out << "ShScalarToReducedGGPLGiven[";
    ShToGridded::print(out);
    out << ",pl=" << pl_.size()
        << "]";
}


const char* ShScalarToReducedGGPLGiven::name() const {
    return "ShScalarToReducedGGPLGiven";
}


const repres::Representation* ShScalarToReducedGGPLGiven::outputRepresentation() const {
    return new repres::gauss::reduced::ReducedFromPL(pl_);
}


namespace {
static ActionBuilder< ShScalarToReducedGGPLGiven > __action("transform.sh-scalar-to-reduced-gg-pl-given");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

