// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/util/LonLatToXYZ.h"

#include "eckit/geo/Figure.h"
#include "eckit/geo/projection/LonLatToXYZ.h"

#include "mir/util/Exceptions.h"


namespace mir::util {


LonLatToXYZ::LonLatToXYZ(const eckit::geo::Figure& figure, double poleDisplacement) :
    to_xyz_(std::make_unique<eckit::geo::projection::LonLatToXYZ>(eckit::geo::FigureFactory::build(figure))),
    poleDisplacement_(poleDisplacement) {
    ASSERT(poleDisplacement_ >= 0.);
}


LonLatToXYZ::~LonLatToXYZ() = default;


PointXYZ LonLatToXYZ::operator()(const PointLonLat& p) const {
    const auto& eps = poleDisplacement_;
    const auto lat  = Latitude::NORTH_POLE <= p.lat() + eps   ? p.lat() - eps
                      : Latitude::SOUTH_POLE >= p.lat() - eps ? p.lat() + eps
                                                              : p.lat();
    return to_xyz_->fwd(PointLonLat{p.lon(), lat});
}


}  // namespace mir::util
