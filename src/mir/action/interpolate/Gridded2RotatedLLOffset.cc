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

#include "mir/action/interpolate/Gridded2RotatedLLOffset.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"


#include "mir/repres/latlon/RotatedLLOffset.h"

#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace action {


Gridded2RotatedLLOffset::Gridded2RotatedLLOffset(const param::MIRParametrisation &parametrisation):
    Gridded2GriddedInterpolation(parametrisation) {

      NOTIMP;

    std::vector<double> value;
    ASSERT(parametrisation_.get("user.grid", value));
    ASSERT(value.size() == 2);
    increments_ = util::Increments(value[0], value[1]);

    ASSERT(parametrisation_.get("user.rotation", value));
    ASSERT(value.size() == 2);

    rotation_ = util::Rotation(value[0], value[1]);

    ASSERT(parametrisation_.get("user.shift", value));
    ASSERT(value.size() == 2);

    eastwards_ = value[0];
    northwards_ = value[1];

}


Gridded2RotatedLLOffset::~Gridded2RotatedLLOffset() {
}


bool Gridded2RotatedLLOffset::sameAs(const Action& other) const {
    const Gridded2RotatedLLOffset* o = dynamic_cast<const Gridded2RotatedLLOffset*>(&other);
    return o && (increments_ == o->increments_)
           && (rotation_ == o->rotation_)
           && (northwards_ == o->northwards_)
           && (eastwards_ == o->eastwards_);
}

void Gridded2RotatedLLOffset::print(std::ostream &out) const {
    out << "Gridded2RotatedLLOffset[increments=" << increments_
        << ",rotation=" << rotation_
        << ",northwards=" << northwards_
        << ",eastwards=" << eastwards_ << "]";
}


const repres::Representation *Gridded2RotatedLLOffset::outputRepresentation() const {
    return new repres::latlon::RotatedLLOffset(
               util::BoundingBox(90, 0, -90, 360 - increments_.west_east()),
               increments_,
               rotation_,
               northwards_,
               eastwards_);
}


namespace {
static ActionBuilder< Gridded2RotatedLLOffset > grid2grid("interpolate.grid2rotated-regular-ll-offset");
}


}  // namespace action
}  // namespace mir

