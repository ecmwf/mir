// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/util/LonLatToPoint3.h"

#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"


namespace mir::util {


LonLatToPoint3::LonLatToPoint3(const repres::Representation& rep, double poleDisplacement) {
    if (poleDisplacement > 0. && (rep.includesNorthPole() || rep.includesSouthPole())) {
        poleDisplacement_ = poleDisplacement;
    }
}


PointXYZ LonLatToPoint3::operator()(const PointLonLat& p) const {
    const auto& eps = poleDisplacement_;
    const auto lat  = Latitude::NORTH_POLE <= p.lat() + eps   ? p.lat() - eps
                      : Latitude::SOUTH_POLE >= p.lat() - eps ? p.lat() + eps
                                                              : p.lat();
    return repres::Iterator::point_3D(PointLonLat{p.lon(), lat});
}


}  // namespace mir::util
