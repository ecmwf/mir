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


#include "mir/util/Point2ToPointXYZ.h"

#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"


namespace mir::util {


Point2ToPointXYZ::Point2ToPointXYZ(const repres::Representation& rep, double poleDisplacement) :
    calculate_(poleDisplacement > 0. && (rep.includesNorthPole() || rep.includesSouthPole())
                   ? static_cast<Calculate*>(new PointXYZDisplaced(poleDisplacement))
                   : new PointXYZSimple) {}


PointXYZ Point2ToPointXYZ::operator()(const PointXY& p2) const {
    return calculate_->make_point3(p2);
}


Point2ToPointXYZ::PointXYZDisplaced::PointXYZDisplaced(double poleDisplacement) : eps_(poleDisplacement) {
    ASSERT(eps_ > 0.);
}


PointXYZ Point2ToPointXYZ::PointXYZDisplaced::make_point3(const PointXY& p2) const {
    const PointXY p2d{Latitude::NORTH_POLE <= p2[0] + eps_   ? p2[0] - eps_
                      : Latitude::SOUTH_POLE >= p2[0] - eps_ ? p2[0] + eps_
                                                             : p2[0],
                      p2[1]};
    return repres::Iterator::point_3D(p2d);
}


PointXYZ Point2ToPointXYZ::PointXYZSimple::make_point3(const PointXY& p2) const {
    return repres::Iterator::point_3D(p2);
}


}  // namespace mir::util
