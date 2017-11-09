/*
 * (C) Copyright 1996-2017 ECMWF.
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


#include "mir/action/transform/ShScalarToReducedGG.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/ReducedClassic.h"


namespace mir {
namespace action {
namespace transform {


ShScalarToReducedGG::ShScalarToReducedGG(const param::MIRParametrisation& parametrisation):
    ShScalarToGridded(parametrisation) {

    ASSERT(parametrisation_.userParametrisation().get("reduced", N_));

}


ShScalarToReducedGG::~ShScalarToReducedGG() {
}


bool ShScalarToReducedGG::sameAs(const Action& other) const {
    const ShScalarToReducedGG* o = dynamic_cast<const ShScalarToReducedGG*>(&other);
    return o && (N_ == o->N_);
}


void ShScalarToReducedGG::print(std::ostream& out) const {
    out << "ShScalarToReducedGG[N=" << N_ << "]";
}


const repres::Representation* ShScalarToReducedGG::outputRepresentation() const {
    return new repres::gauss::reduced::ReducedClassic(N_);
}


namespace {
static ActionBuilder< ShScalarToReducedGG > __action("transform.sh-scalar-to-reduced-gg");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

