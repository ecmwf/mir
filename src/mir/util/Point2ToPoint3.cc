/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/util/Point2ToPoint3.h"

#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"


namespace mir::util {


Point2ToPoint3::Point2ToPoint3(const repres::Representation& rep, double poleDisplacement) :
    calculate_(poleDisplacement > 0. && (rep.includesNorthPole() || rep.includesSouthPole())
                   ? static_cast<Calculate*>(new Point3Displaced(poleDisplacement))
                   : new Point3Simple) {}


Point3 Point2ToPoint3::operator()(const Point2& p2) const {
    return calculate_->make_point3(p2);
}


Point2ToPoint3::Point3Displaced::Point3Displaced(double poleDisplacement) : eps_(poleDisplacement) {
    ASSERT(eps_ > 0.);
}


Point3 Point2ToPoint3::Point3Displaced::make_point3(const Point2& p2) const {
    const Point2 p2d{p2[0], Latitude::NORTH_POLE == p2[1]   ? p2[1] - eps_
                            : Latitude::SOUTH_POLE == p2[1] ? p2[1] + eps_
                                                            : p2[1]};
    return repres::Iterator::point_3D(p2d);
}


Point3 Point2ToPoint3::Point3Simple::make_point3(const Point2& p2) const {
    return repres::Iterator::point_3D(p2);
}


}  // namespace mir::util
