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

#include "mir/util/LatitudeIncrement.h"
#include "mir/util/LongitudeIncrement.h"


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
    // -- Exceptions
    // None

    // -- Constructors

    Increments();
    Increments(const param::MIRParametrisation&);
    Increments(const Increments&);
    explicit Increments(double westEastIncrement, double southNorthIncrement);
    explicit Increments(const LongitudeIncrement&, const LatitudeIncrement&);

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

    bool isShifted(const PointLatLon&) const;
    bool isLatitudeShifted(const PointLatLon&) const;
    bool isLongitudeShifted(const PointLatLon&) const;

    const LongitudeIncrement& west_east() const { return west_east_; }

    const LatitudeIncrement& south_north() const { return south_north_; }

    void fillGrib(grib_info&) const;

    void fillJob(api::MIRJob&) const;

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

    LongitudeIncrement west_east_;
    LatitudeIncrement south_north_;

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
