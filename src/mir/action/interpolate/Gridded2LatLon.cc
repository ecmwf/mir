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

#include "eckit/exception/Exceptions.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace action {


Gridded2LatLon::Gridded2LatLon(const param::MIRParametrisation& parametrisation):
    Gridded2GriddedInterpolation(parametrisation) {

    std::vector<double> value;
    ASSERT(parametrisation_.userParametrisation().get("grid", value));

    ASSERT(value.size() == 2);
    increments_ = util::Increments(value[0], value[1]);

    // TODO: maybe use 'user.autoshift'
    if (parametrisation_.userParametrisation().get("area", value)) {
        ASSERT(value.size() == 4);
        bbox_ = util::BoundingBox(value[0], value[1], value[2], value[3]);
    }

    increments_.globaliseBoundingBox(bbox_);
    eckit::Log::debug<LibMir>()
            << "Gridded2LatLon: globalise:"
            << " " << bbox_
            << " " << increments_
            << " Shifted? " << (increments_.isShifted(bbox_) ? "yes" : "no")
            << std::endl;
}


Gridded2LatLon::~Gridded2LatLon() {
}


bool Gridded2LatLon::sameAs(const Action& other) const {
    const Gridded2LatLon* o = dynamic_cast<const Gridded2LatLon*>(&other);
    return o  && (increments_ == o->increments_) && (bbox_ == o->bbox_) && Gridded2GriddedInterpolation::sameAs(other);
}

void Gridded2LatLon::print(std::ostream& out) const {
    out << "increments="
        << increments_
        << ",bbox="
        << bbox_
        << ",";
    Gridded2GriddedInterpolation::print(out);
}


}  // namespace action
}  // namespace mir

