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


#include "mir/action/transform/ScalarSh2Octahedral.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/gauss/reduced/ReducedOctahedral.h"


namespace mir {
namespace action {
namespace transform {


ScalarSh2Octahedral::ScalarSh2Octahedral(const param::MIRParametrisation& parametrisation):
    ScalarSh2Gridded(parametrisation) {

    ASSERT(parametrisation_.get("user.octahedral", N_));

}


bool ScalarSh2Octahedral::sameAs(const Action& other) const {
    const ScalarSh2Octahedral* o = dynamic_cast<const ScalarSh2Octahedral*>(&other);
    return o && (N_ == o->N_);
}


ScalarSh2Octahedral::~ScalarSh2Octahedral() {
}


void ScalarSh2Octahedral::print(std::ostream& out) const {
    out << "ScalarSh2Octahedral[N=" << N_ << "]";
}


const repres::Representation* ScalarSh2Octahedral::outputRepresentation() const {
    return new repres::reduced::ReducedOctahedral(N_);
}


namespace {
static ActionBuilder< ScalarSh2Octahedral > __action("transform.scalar-sh2octahedral-gg");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

