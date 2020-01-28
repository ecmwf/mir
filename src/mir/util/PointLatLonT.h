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


#ifndef mir_util_PointLatLonT_h
#define mir_util_PointLatLonT_h

#include <iostream>


namespace mir {
namespace util {


template<typename LAT, typename LON>
class PointLatLonT {
public:

    PointLatLonT(const LAT& lat, const LON& lon) : lat_(lat), lon_(lon) {}
    PointLatLonT() = default;
    PointLatLonT(const PointLatLonT&) = default;
    PointLatLonT(PointLatLonT&&) noexcept = default;

    PointLatLonT& operator=(const PointLatLonT&) = default;

    const LAT& lat() const { return lat_; }
    const LON& lon() const { return lon_; }

    LAT& lat(const LAT& lat) { return (lat_ = lat); }
    LON& lon(const LON& lon)  { return (lon_ = lon); }

protected:

    LAT lat_;
    LON lon_;

    virtual void print(std::ostream& out) const {
        out << "PointLatLon[" << lat_ << "," << lon_ << "]";
    }

private:

    friend std::ostream& operator<<(std::ostream& s, const PointLatLonT& p) {
        p.print(s);
        return s;
    }

};


}  // namespace util
}  // namespace mir


#endif
