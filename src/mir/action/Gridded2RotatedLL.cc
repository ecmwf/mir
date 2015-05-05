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

#include "mir/action/Gridded2RotatedLL.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"


#include "mir/repres/RotatedLL.h"

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

    rotation_ = util::Rotation(value[0], value[1]);
}


Gridded2RotatedLL::~Gridded2RotatedLL() {
}


void Gridded2RotatedLL::print(std::ostream &out) const {
    out << "Gridded2RotatedLL[increments=" << increments_ << "]";
}


repres::Representation *Gridded2RotatedLL::outputRepresentation(const repres::Representation *inputRepres) const {
    return new repres::RotatedLL(
               util::BoundingBox(90, 0, -90, 360 - increments_.west_east()),
               increments_, rotation_);
}


namespace {
static ActionBuilder< Gridded2RotatedLL > grid2grid("interpolate.grid2rotated-ll");
}


}  // namespace action
}  // namespace mir

