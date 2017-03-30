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

#include "mir/action/interpolate/Gridded2RegularLLShift.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "eckit/types/Fraction.h"


#include "mir/repres/latlon/RegularLLShift.h"

#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace action {


Gridded2RegularLLShift::Gridded2RegularLLShift(const param::MIRParametrisation &parametrisation):
    Gridded2GriddedInterpolation(parametrisation) {

    std::vector<double> value;
    ASSERT(parametrisation_.get("user.grid", value));
    ASSERT(value.size() == 2);
    increments_ = util::Increments(value[0], value[1]);

    ASSERT(parametrisation_.get("user.area", value));
    ASSERT(value.size() == 4);

    ASSERT(parametrisation_.get("user.shift", value));
    ASSERT(value.size() == 2);

    shift_ = util::Shift(value[0], value[1]);

}


Gridded2RegularLLShift::~Gridded2RegularLLShift() {
}


bool Gridded2RegularLLShift::sameAs(const Action& other) const {
    const Gridded2RegularLLShift* o = dynamic_cast<const Gridded2RegularLLShift*>(&other);
    return o && (increments_ == o->increments_) && (shift_ == o->shift_);
}

void Gridded2RegularLLShift::print(std::ostream &out) const {
    out << "Gridded2RegularLLShift[increments=" << increments_
        << ",shift=" << shift_
        << "]";
}


const repres::Representation *Gridded2RegularLLShift::outputRepresentation() const {
    
    eckit::Fraction we(increments_.west_east());
    eckit::Fraction sn(increments_.south_north());
    
    double north = 90 - (90/sn).decimalPart() * sn;
    double south = -90 -(-90/sn).decimalPart() * sn;
    
    double west = 0;
    double east = 360 -(360/we).decimalPart() * we;
    
    util::BoundingBox bbox(north + shift_.south_north(),
                           west + shift_.west_east(),
                           south + shift_.south_north(),
                           east - increments_.west_east() + shift_.west_east());
    
    
    
    return new repres::latlon::RegularLLShift(bbox,
                                              increments_,
                                              shift_);
}


namespace {
static ActionBuilder< Gridded2RegularLLShift > grid2grid("interpolate.grid2regular-ll-shift");
}


}  // namespace action
}  // namespace mir

