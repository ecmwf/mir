// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>

#include "eckit/geo/grid/regular/RegularLL.h"

#include "mir/util/Types.h"


namespace eckit {
class JSON;
namespace spec {
class Custom;
}  // namespace spec
}  // namespace eckit

struct grib_info;
namespace mir {
namespace param {
class MIRParametrisation;
}
namespace api {
class MIRJob;
}
}  // namespace mir
namespace mir::util {
class BoundingBox;
}  // namespace mir::util


namespace mir::util {


class Increments {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Increments();
    Increments(const param::MIRParametrisation&);
    Increments(const Increments&);
    explicit Increments(double westEastIncrement, double southNorthIncrement);

    // -- Destructor

    ~Increments() = default;

    // -- Convertors
    // None

    // -- Operators

    bool operator==(const Increments&) const;

    bool operator!=(const Increments&) const;

    Increments& operator=(const Increments&);

    // -- Methods

    bool isPeriodic() const;

    bool isShifted(const BoundingBox&) const;
    bool isLatitudeShifted(const BoundingBox&) const;
    bool isLongitudeShifted(const BoundingBox&) const;

    bool isShifted(const PointLonLat&) const;
    bool isLatitudeShifted(const PointLonLat&) const;
    bool isLongitudeShifted(const PointLonLat&) const;

    // transitional, while mir's Latitude/Longitude are in use
    bool isShifted(const PointLatLon& p) const { return isShifted(p.lonlat()); }
    bool isLatitudeShifted(const PointLatLon& p) const { return isLatitudeShifted(p.lonlat()); }
    bool isLongitudeShifted(const PointLatLon& p) const { return isLongitudeShifted(p.lonlat()); }

    double west_east() const { return inc_.dlon(); }

    double south_north() const { return inc_.dlat(); }

    void fillGrib(grib_info&) const;

    void fillJob(api::MIRJob&) const;
    void fillSpec(eckit::spec::Custom&) const;

    void makeName(std::ostream&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members
    // None

    // -- Methods

    void print(std::ostream&) const;
    void json(eckit::JSON&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    eckit::geo::grid::regular::RegularLL::Increments inc_;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const Increments& p) {
        p.print(s);
        return s;
    }

    friend eckit::JSON& operator<<(eckit::JSON& s, const Increments& p) {
        p.json(s);
        return s;
    }
};


}  // namespace mir::util
