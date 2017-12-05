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


#include "mir/action/transform/ShScalarToOctahedralGG.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/ReducedOctahedral.h"


namespace mir {
namespace action {
namespace transform {


ShScalarToOctahedralGG::ShScalarToOctahedralGG(const param::MIRParametrisation& parametrisation):
    ShScalarToGridded(parametrisation) {

    ASSERT(parametrisation_.userParametrisation().get("octahedral", N_));

}


bool ShScalarToOctahedralGG::sameAs(const Action& other) const {
    const ShScalarToOctahedralGG* o = dynamic_cast<const ShScalarToOctahedralGG*>(&other);
    return o && (N_ == o->N_);
}


ShScalarToOctahedralGG::~ShScalarToOctahedralGG() {
}


void ShScalarToOctahedralGG::print(std::ostream& out) const {
    out << "ShScalarToOctahedralGG[N=" << N_ << "]";
}


const repres::Representation* ShScalarToOctahedralGG::outputRepresentation() const {
    return new repres::gauss::reduced::ReducedOctahedral(N_);
}

const char* ShScalarToOctahedralGG::name() const {
    return "ShScalarToOctahedralGG";
}


namespace {
static ActionBuilder< ShScalarToOctahedralGG > __action("transform.sh-scalar-to-octahedral-gg");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

