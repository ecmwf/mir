/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_util_Domain_h
#define mir_util_Domain_h

#include <iostream>

#include <iosfwd>
#include "mir/util/Types.h"


namespace atlas {
class RectangularDomain;
}


namespace mir {
namespace util {


class Domain {
public:

    // -- Exceptions
    // None

    // -- Contructors

    /// ctor using coordinates
    explicit Domain(Latitude north = Latitude::NORTH_POLE,
                    Longitude west = Longitude::GREENWICH,
                    Latitude south = Latitude::SOUTH_POLE,
                    Longitude east = Longitude::GLOBE);

    // -- Methods

    /// Generator for a global Domain
    static Domain makeGlobal();

    operator atlas::RectangularDomain() const;

    const Latitude& north() const {
        return north_;
    }

    const Longitude& west() const {
        return west_;
    }

    const Latitude& south() const {
        return south_;
    }

    const Longitude& east() const {
        return east_;
    }

    bool contains(const Latitude& lat, const Longitude& lon) const;

    /// Check if grid includes the North pole
    bool includesPoleNorth() const {
        return north() == Latitude::NORTH_POLE;
    }

    /// Check if grid includes the South pole
    bool includesPoleSouth() const {
        return south() == Latitude::SOUTH_POLE;
    }

    /// Check if grid spans the complete range East-West (periodic)
    bool isPeriodicEastWest() const {
        return east() - west() == Longitude::GLOBE;
    }

    /// Check if domain represents the complete globe surface
    bool isGlobal() const {
        return includesPoleNorth() && includesPoleSouth() && isPeriodicEastWest();
    }

    /// Output to stream
    void print(std::ostream&) const;

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
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None

private:

    // -- Members

    Latitude north_;
    Longitude west_;
    Latitude south_;
    Longitude east_;

    // -- Methods

    void normalise();
    void check();

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    /// Output using stream operator
    friend std::ostream &operator<<(std::ostream &s, const Domain &p) {
        p.print(s);
        return s;
    }

};


} // namespace util
} // namespace mir


#endif
