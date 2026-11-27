// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <initializer_list>
#include <memory>

#include "mir/util/Types.h"


namespace eckit::geo {
class Figure;
namespace projection {
class LonLatToXYZ;
}
}  // namespace eckit::geo


namespace mir::util {


/// Points in 3D on a figure (of the points searched, as eckit::geo::Search), displaced from the poles (if not 0), as
/// points on the poles are coincident in 3D whatever their longitude
class LonLatToXYZ final {
public:
    explicit LonLatToXYZ(const eckit::geo::Figure&, double poleDisplacement = 0.);

    LonLatToXYZ(const LonLatToXYZ&) = delete;
    LonLatToXYZ(LonLatToXYZ&&)      = delete;

    ~LonLatToXYZ();

    LonLatToXYZ& operator=(const LonLatToXYZ&) = delete;
    LonLatToXYZ& operator=(LonLatToXYZ&&)      = delete;

    PointXYZ operator()(const PointLonLat&) const;

    /// A braced list doesn't name its order (latitude/longitude or longitude/latitude), so the point is explicit
    PointXYZ operator()(std::initializer_list<double>) const = delete;

private:
    std::unique_ptr<const eckit::geo::projection::LonLatToXYZ> to_xyz_;
    double poleDisplacement_;
};


}  // namespace mir::util
