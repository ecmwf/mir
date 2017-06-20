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


#include "mir/action/interpolate/Gridded2RegularLL.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "eckit/types/Fraction.h"
#include "mir/repres/latlon/RegularLL.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace action {


Gridded2RegularLL::Gridded2RegularLL(const param::MIRParametrisation& parametrisation):
    Gridded2GriddedInterpolation(parametrisation) {

    std::vector<double> value;
    ASSERT(parametrisation_.get("user.grid", value));
    ASSERT(value.size() == 2);
    increments_ = util::Increments(value[0], value[1]);

    if (parametrisation_.get("user.shift", value)) {
        ASSERT(value.size() == 2);
        shift_ = util::Shift(value[0], value[1]);
    }

}


Gridded2RegularLL::~Gridded2RegularLL() {
}


bool Gridded2RegularLL::sameAs(const Action& other) const {
    const Gridded2RegularLL* o = dynamic_cast<const Gridded2RegularLL*>(&other);
    return o && (increments_ == o->increments_) && (shift_ == o->shift_);
}


void Gridded2RegularLL::print(std::ostream& out) const {
    out << "Gridded2RegularLL["
            "increments=" << increments_
        << ",shift=" << shift_
        << "]";
}


static eckit::Fraction NORTH_POLE(90);
static eckit::Fraction SOUTH_POLE(-90);
static eckit::Fraction ZERO(0);
static eckit::Fraction THREE_SIXTY(360);

static eckit::Fraction adjust(eckit::Fraction lat, const eckit::Fraction& sn) {
    while(lat > NORTH_POLE) { lat -= sn; }
    while(lat < SOUTH_POLE) { lat += sn; }
    return lat;
}


const repres::Representation* Gridded2RegularLL::outputRepresentation() const {
    using eckit::Fraction;

    // Latitude range: cater for grids that are regular, but do not reach the pole (e.g. 1.6)
    Fraction north = adjust(NORTH_POLE + shift_.south_north(), increments_.south_north());
    Fraction south = adjust(SOUTH_POLE + shift_.south_north(), increments_.south_north());

    Fraction west = ZERO + shift_.west_east();
    Fraction east = THREE_SIXTY+ shift_.west_east() - increments_.west_east();

    return new repres::latlon::RegularLL(
               util::BoundingBox(north, west, south, east),
               increments_,
               shift_);
}


namespace {
static ActionBuilder< Gridded2RegularLL > grid2grid("interpolate.grid2regular-ll");
}


}  // namespace action
}  // namespace mir

