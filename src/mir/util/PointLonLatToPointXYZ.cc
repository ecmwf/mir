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


#include "mir/util/PointLonLatToPointXYZ.h"

#include "eckit/geo/projection/LonLatToXYZ.h"

#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"


namespace mir::util {


static eckit::geo::projection::LonLatToXYZ to_xyz;


PointLonLatToPointXYZ::PointLonLatToPointXYZ(const repres::Representation& rep, double poleDisplacement) :
    calculate_(poleDisplacement > 0. && (rep.includesNorthPole() || rep.includesSouthPole())
                   ? static_cast<Calculate*>(new PointXYZDisplaced(poleDisplacement))
                   : new PointXYZSimple) {}


PointXYZ PointLonLatToPointXYZ::fwd(const PointLonLat& p) const {
    return calculate_->make_point3(p);
}


PointLonLat PointLonLatToPointXYZ::inv(const PointXYZ& p) const {
    return to_xyz.inv(p);
}


PointLonLatToPointXYZ::PointXYZDisplaced::PointXYZDisplaced(double poleDisplacement) : eps_(poleDisplacement) {
    ASSERT(eps_ > 0.);
}


PointXYZ PointLonLatToPointXYZ::PointXYZDisplaced::make_point3(const PointLonLat& p) const {
    return to_xyz.fwd(p.north_pole()   ? PointLonLat{p.lon, PointLonLat::RIGHT_ANGLE - eps_}
                      : p.south_pole() ? PointLonLat{p.lon, -PointLonLat::RIGHT_ANGLE + eps_}
                                       : p);
}


PointXYZ PointLonLatToPointXYZ::PointXYZSimple::make_point3(const PointLonLat& p) const {
    return to_xyz.fwd(p);
}


}  // namespace mir::util
