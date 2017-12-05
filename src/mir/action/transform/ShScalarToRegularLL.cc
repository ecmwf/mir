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


#include "mir/action/transform/ShScalarToRegularLL.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/latlon/RegularLL.h"


namespace mir {
namespace action {
namespace transform {


ShScalarToRegularLL::ShScalarToRegularLL(const param::MIRParametrisation &parametrisation):
    ShScalarToGridded(parametrisation) {

    std::vector<double> value;
    ASSERT(parametrisation_.userParametrisation().get("grid", value));
    ASSERT(value.size() == 2);
    increments_ = util::Increments(value[0], value[1]);

}


ShScalarToRegularLL::~ShScalarToRegularLL() {
}


bool ShScalarToRegularLL::sameAs(const Action& other) const {
    const ShScalarToRegularLL* o = dynamic_cast<const ShScalarToRegularLL*>(&other);
    return o && (increments_ == o->increments_);
}


void ShScalarToRegularLL::print(std::ostream &out) const {
    out << "ShScalarToRegularLL[increments=" << increments_ << "]";
}


const repres::Representation *ShScalarToRegularLL::outputRepresentation() const {

    if (!increments_.isPeriodic()) {
        throw eckit::UserError("Spectral transforms only support periodic regular grids", Here());
    }

    // use (non-shifted) global bounding box
    util::BoundingBox bbox;
    increments_.globaliseBoundingBox(bbox, false, false);

    return new repres::latlon::RegularLL(bbox, increments_);
}

const char* ShScalarToRegularLL::name() const {
    return "ShScalarToRegularLL";
}

namespace {
static ActionBuilder< ShScalarToRegularLL > __action("transform.sh-scalar-to-regular-ll");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

