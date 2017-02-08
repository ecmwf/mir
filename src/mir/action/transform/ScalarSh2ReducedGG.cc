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


#include "mir/action/transform/ScalarSh2ReducedGG.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/ReducedClassic.h"


namespace mir {
namespace action {
namespace transform {


ScalarSh2ReducedGG::ScalarSh2ReducedGG(const param::MIRParametrisation& parametrisation):
    Sh2Gridded(parametrisation) {

    ASSERT(parametrisation_.get("user.reduced", N_));

}


ScalarSh2ReducedGG::~ScalarSh2ReducedGG() {
}


bool ScalarSh2ReducedGG::sameAs(const Action& other) const {
    const ScalarSh2ReducedGG* o = dynamic_cast<const ScalarSh2ReducedGG*>(&other);
    return o && (N_ == o->N_);
}


void ScalarSh2ReducedGG::print(std::ostream& out) const {
    out << "ScalarSh2ReducedGG[N=" << N_ << "]";
}


const repres::Representation* ScalarSh2ReducedGG::outputRepresentation() const {
    return new repres::reduced::ReducedClassic(N_);
}


namespace {
static ActionBuilder< ScalarSh2ReducedGG > __action("transform.scalar-sh2reduced-gg");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

