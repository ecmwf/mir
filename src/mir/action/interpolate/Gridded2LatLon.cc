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


#include "mir/action/interpolate/Gridded2LatLon.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "eckit/types/Fraction.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/latlon/RegularLL.h"


namespace mir {
namespace action {


Gridded2LatLon::Gridded2LatLon(const param::MIRParametrisation& parametrisation):
    Gridded2GriddedInterpolation(parametrisation) {

    std::vector<double> value;
    ASSERT(parametrisation_.get("user.grid", value));
    ASSERT(value.size() == 2);
    increments_ = util::Increments(value[0], value[1]);

    // TODO: maybe use 'user.autoshift'
    if (parametrisation_.get("user.area", value)) {
        ASSERT(value.size() == 4);

        util::BoundingBox bbox(value[0], value[1], value[2], value[3]);

        shift_ = increments_.shiftFromZeroZero(bbox);
        if (shift_) {
            eckit::Log::info() << "Shifting grid "
                               << bbox
                               << " "
                               << increments_
                               << " => "
                               << shift_
                               << std::endl;
        }


    }

    bbox_ = increments_.globalBoundingBox(shift_);
}


Gridded2LatLon::~Gridded2LatLon() {
}


bool Gridded2LatLon::sameAs(const Action& other) const {
    const Gridded2LatLon* o = dynamic_cast<const Gridded2LatLon*>(&other);
    return o && (increments_ == o->increments_)
            && (shift_ == o->shift_)
            && (bbox_ == o->bbox_);
}



}  // namespace action
}  // namespace mir

