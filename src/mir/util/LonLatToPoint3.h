// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <initializer_list>

#include "mir/util/Types.h"


namespace mir::repres {
class Representation;
}


namespace mir::util {


/// Point in 3D (see repres::Iterator::point_3D), displaced from the poles if the representation includes them, as
/// points on the poles are coincident in 3D whatever their longitude
class LonLatToPoint3 final {
public:
    LonLatToPoint3(const repres::Representation&, double poleDisplacement);

    PointXYZ operator()(const PointLonLat&) const;

    /// A braced list doesn't name its order (latitude/longitude or longitude/latitude), so the point is explicit
    PointXYZ operator()(std::initializer_list<double>) const = delete;

private:
    double poleDisplacement_ = 0.;  // none, if 0
};


}  // namespace mir::util
