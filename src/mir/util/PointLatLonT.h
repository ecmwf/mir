// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <ostream>

#include "eckit/geo/PointLonLat.h"


namespace mir::util {


template <typename LAT, typename LON>
class PointLatLonT {
public:
    PointLatLonT(const LAT& lat, const LON& lon) : lat_(lat), lon_(lon) {}
    explicit PointLatLonT(const eckit::geo::PointLonLat& p) : lat_(p.lat()), lon_(p.lon()) {}
    PointLatLonT() = default;

    const LAT& lat() const { return lat_; }
    const LON& lon() const { return lon_; }

    LAT& lat(const LAT& lat) { return (lat_ = lat); }
    LON& lon(const LON& lon) { return (lon_ = lon); }

    /// The point, of the coordinates as kept
    eckit::geo::PointLonLat lonlat() const { return {lon_.value(), lat_.value()}; }

private:
    LAT lat_;
    LON lon_;

    friend std::ostream& operator<<(std::ostream& out, const PointLatLonT& p) {
        return out << "PointLatLon[" << p.lat_ << "," << p.lon_ << "]";
    }
};


}  // namespace mir::util
