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
#include "mir/util/BoundingBox.h"

namespace eckit {
class MD5;
}

namespace mir {
namespace util {


class Domain : public BoundingBox {
public:

    // -- Exceptions
    // None

    // -- Contructors

    /// ctor using coordinates
    Domain(BoundingBox::value_type north,
           BoundingBox::value_type west,
           BoundingBox::value_type south,
           BoundingBox::value_type east )
        : BoundingBox(north, west, south, east) {
    }

    // -- Methods

    /// Generator for a global Domain
    static Domain makeGlobal() { return Domain(NORTH_POLE, ZERO,SOUTH_POLE, THREE_SIXTY); }

    /// Generator for an empty Domain
    static Domain makeEmpty() { return Domain(ZERO, ZERO, ZERO, ZERO); }

    /// Check if grid includes the North pole
    bool includesPoleNorth() const { return north() == NORTH_POLE; }

    /// Check if grid includes the South pole
    bool includesPoleSouth() const { return south() == SOUTH_POLE; }

    /// Check if grid spans the complete range East-West (periodic)
    bool isPeriodicEastWest() const { return east() - west() == THREE_SIXTY; }

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

    /// Output using stream operator
    friend std::ostream &operator<<(std::ostream &s, const Domain &p) {
        p.print(s);
        return s;
    }
};


} // namespace util
} // namespace mir


#endif
