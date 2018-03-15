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


#include "mir/action/transform/ShScalarToRegularGG.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/regular/RegularGG.h"


namespace mir {
namespace action {
namespace transform {


ShScalarToRegularGG::ShScalarToRegularGG(const param::MIRParametrisation& parametrisation):
    ShScalarToGridded(parametrisation) {
    ASSERT(parametrisation_.userParametrisation().get("regular", N_));
}


ShScalarToRegularGG::~ShScalarToRegularGG() {
}


bool ShScalarToRegularGG::sameAs(const Action& other) const {
    const ShScalarToRegularGG* o = dynamic_cast<const ShScalarToRegularGG*>(&other);
    return o && (N_ == o->N_);
}


void ShScalarToRegularGG::print(std::ostream& out) const {
    out << "ShScalarToRegularGG[";
    ShToGridded::print(out);
    out << ",N=" << N_
        << "]";
}


void ShScalarToRegularGG::custom(std::ostream& out) const {
    out << "ShScalarToRegularGG[";
    ShToGridded::custom(out);
    out << ",N=" << N_
        << "]";
}


const char* ShScalarToRegularGG::name() const {
    return "ShScalarToRegularGG";
}


const repres::Representation* ShScalarToRegularGG::outputRepresentation() const {
    return new repres::gauss::regular::RegularGG(N_);
}


namespace {
static ActionBuilder< ShScalarToRegularGG > __action("transform.sh-scalar-to-regular-gg");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

