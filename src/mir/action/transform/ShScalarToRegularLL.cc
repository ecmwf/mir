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
    ASSERT(parametrisation_.get("user.grid", value));
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
    double ns = increments_.south_north();
    double we = increments_.west_east();

    // Latitude range: cater for grids that are regular, but do not reach the pole (e.g. 1.6)
    double pole = size_t(90 / ns) * ns;

    // Longitude range
    // - periodic grids have East-most longitude at 360 - increment
    // - non-periodic grids are symmetric to Greenwhich and do not reach the date line (e.g. 1.1)
    double west = 0;
    double east = size_t(360 / we) * we;
    if (east == 360) {
        east -= we;
    }
    else {
        east = size_t(180 / we) * we;
        west = -east;
    }

    return new repres::latlon::RegularLL(
                util::BoundingBox(pole, west, -pole, east),
                increments_,
                util::Shift(0, 0));
}


namespace {
static ActionBuilder< ShScalarToRegularLL > __action("transform.sh-scalar-to-regular-ll");
}


}  // namespace transform
}  // namespace action
}  // namespace mir

