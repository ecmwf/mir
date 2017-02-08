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


#include "mir/action/transform/ScalarSh2RegularGG.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/regular/RegularGG.h"


namespace mir {
namespace action {
namespace transform {


ScalarSh2RegularGG::ScalarSh2RegularGG(const param::MIRParametrisation& parametrisation):
    Sh2Gridded(parametrisation) {
    ASSERT(parametrisation_.get("user.regular", N_));
}


ScalarSh2RegularGG::~ScalarSh2RegularGG() {
}


bool ScalarSh2RegularGG::sameAs(const Action& other) const {
    const ScalarSh2RegularGG* o = dynamic_cast<const ScalarSh2RegularGG*>(&other);
    return o && (N_ == o->N_);
}


void ScalarSh2RegularGG::print(std::ostream& out) const {
    out << "ScalarSh2RegularGG[N=" << N_ << "]";
}


const repres::Representation* ScalarSh2RegularGG::outputRepresentation() const {
    return new repres::regular::RegularGG(N_);
}


namespace {
static ActionBuilder< ScalarSh2RegularGG > __action("transform.scalar-sh2regular-gg");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

