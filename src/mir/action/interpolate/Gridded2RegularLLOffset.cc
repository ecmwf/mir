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

#include "mir/action/interpolate/Gridded2RegularLLOffset.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"


#include "mir/repres/latlon/RegularLLOffset.h"

#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace action {


Gridded2RegularLLOffset::Gridded2RegularLLOffset(const param::MIRParametrisation &parametrisation):
    Gridded2GriddedInterpolation(parametrisation) {

    std::vector<double> value;
    ASSERT(parametrisation_.get("user.grid", value));
    ASSERT(value.size() == 2);
    increments_ = util::Increments(value[0], value[1]);

    ASSERT(parametrisation_.get("user.area", value));
    ASSERT(value.size() == 4);
    util::BoundingBox bbox(value[0], value[1], value[2], value[3]);
    // We use the north/west corner as a reference. To review.

    double n = ::fabs(bbox.north()) / increments_.south_north();
    northwards_ = n - long(n);

    double e = ::fabs(bbox.west()) / increments_.west_east();
    eastwards_ = e - long(e);


}


Gridded2RegularLLOffset::~Gridded2RegularLLOffset() {
}


bool Gridded2RegularLLOffset::sameAs(const Action& other) const {
    const Gridded2RegularLLOffset* o = dynamic_cast<const Gridded2RegularLLOffset*>(&other);
    return o && (increments_ == o->increments_) && (northwards_ == o->northwards_) && (eastwards_ == o->eastwards_);
}

void Gridded2RegularLLOffset::print(std::ostream &out) const {
    out << "Gridded2RegularLLOffset[increments=" << increments_
        << ",northwards=" << northwards_
        << ",eastwards=" << eastwards_
        << "]";
}


const repres::Representation *Gridded2RegularLLOffset::outputRepresentation() const {
    return new repres::latlon::RegularLLOffset(
               util::BoundingBox(90, 0, -90, 360 - increments_.west_east()),
               increments_,
               northwards_,
               eastwards_ );
}


namespace {
static ActionBuilder< Gridded2RegularLLOffset > grid2grid("interpolate.grid2regular-ll-offset");
}


}  // namespace action
}  // namespace mir

