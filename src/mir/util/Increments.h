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


#pragma once

#include <iosfwd>

#include "mir/util/Types.h"


namespace eckit {
class JSON;
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
    // -- Constructors

    Increments() = default;
    Increments(const param::MIRParametrisation&);

    Increments(const Increments&) = default;
    Increments(Increments&&)      = default;

    explicit Increments(double westEastIncrement, double southNorthIncrement);

    // -- Destructor

    ~Increments() = default;

    // -- Operators

    bool operator==(const Increments&) const;
    bool operator!=(const Increments&) const;

    Increments& operator=(const Increments&) = default;
    Increments& operator=(Increments&&)      = default;

    // -- Methods

    bool isPeriodic() const;

    bool isShifted(const BoundingBox&) const;
    bool isLatitudeShifted(const BoundingBox&) const;
    bool isLongitudeShifted(const BoundingBox&) const;

    bool isShifted(const PointLonLat&) const;
    bool isLatitudeShifted(const PointLonLat&) const;
    bool isLongitudeShifted(const PointLonLat&) const;

    double west_east() const { return west_east_; }
    double south_north() const { return south_north_; }

    void fillGrib(grib_info&) const;

    void fillJob(api::MIRJob&) const;

    void makeName(std::ostream&) const;

protected:
    // -- Methods

    void print(std::ostream&) const;
    void json(eckit::JSON&) const;

private:
    // -- Members

    double west_east_;
    double south_north_;

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
