// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
    const Point2 p2d{Latitude::NORTH_POLE <= p2[0] + eps_   ? p2[0] - eps_
                     : Latitude::SOUTH_POLE >= p2[0] - eps_ ? p2[0] + eps_
                                                            : p2[0],
                     p2[1]};
    return repres::Iterator::point_3D(p2d);
}


Point3 Point2ToPoint3::Point3Simple::make_point3(const Point2& p2) const {
    return repres::Iterator::point_3D(p2);
}


}  // namespace mir::util
