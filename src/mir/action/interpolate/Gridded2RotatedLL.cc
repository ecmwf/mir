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

#include "mir/action/interpolate/Gridded2RotatedLL.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"


#include "mir/repres/latlon/RotatedLL.h"

#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace action {


Gridded2RotatedLL::Gridded2RotatedLL(const param::MIRParametrisation &parametrisation):
    Gridded2GriddedInterpolation(parametrisation) {

    std::vector<double> value;
    ASSERT(parametrisation_.get("user.grid", value));
    ASSERT(value.size() == 2);
    increments_ = util::Increments(value[0], value[1]);

    ASSERT(parametrisation_.get("user.rotation", value));
    ASSERT(value.size() == 2);

    if (parametrisation_.get("user.shift", value)) {
        ASSERT(value.size() == 2);
        shift_ = util::Shift(value[0], value[1]);
    }

    rotation_ = util::Rotation(value[0], value[1]);
}


Gridded2RotatedLL::~Gridded2RotatedLL() {
}


bool Gridded2RotatedLL::sameAs(const Action& other) const {
    const Gridded2RotatedLL* o = dynamic_cast<const Gridded2RotatedLL*>(&other);
    return o && (increments_ == o->increments_) && (shift_ == o->shift_) && (rotation_ == o->rotation_);
}

void Gridded2RotatedLL::print(std::ostream &out) const {
    out << "Gridded2RotatedLL[increments=" << increments_;
    if (shift_) {
        out << ",shift" << shift_;
    }
    out << ",rotation" << rotation_ << "]";
}


const repres::Representation *Gridded2RotatedLL::outputRepresentation() const {
  ASSERT(!shift_);
    return new repres::latlon::RotatedLL(
               util::BoundingBox(90, 0, -90, 360 - increments_.west_east()),
               increments_,
               shift_,
               rotation_ );
}


namespace {
static ActionBuilder< Gridded2RotatedLL > grid2grid("interpolate.grid2rotated-regular-ll");
}


}  // namespace action
}  // namespace mir

