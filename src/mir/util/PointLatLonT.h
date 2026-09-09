// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <ostream>


namespace mir::util {


template <typename LAT, typename LON>
class PointLatLonT {
public:
    PointLatLonT(const LAT& lat, const LON& lon) : lat_(lat), lon_(lon) {}
    PointLatLonT()                        = default;
    PointLatLonT(const PointLatLonT&)     = default;
    PointLatLonT(PointLatLonT&&) noexcept = default;
    virtual ~PointLatLonT()               = default;

    PointLatLonT& operator=(const PointLatLonT&) = default;

    const LAT& lat() const { return lat_; }
    const LON& lon() const { return lon_; }

    LAT& lat(const LAT& lat) { return (lat_ = lat); }
    LON& lon(const LON& lon) { return (lon_ = lon); }

protected:
    LAT lat_;
    LON lon_;

    virtual void print(std::ostream& out) const { out << "PointLatLon[" << lat_ << "," << lon_ << "]"; }

private:
    friend std::ostream& operator<<(std::ostream& out, const PointLatLonT& p) {
        p.print(out);
        return out;
    }
};


}  // namespace mir::util
