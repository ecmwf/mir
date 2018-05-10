/*
 * (C) Copyright 1996- ECMWF.
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

namespace atlas {
class RectangularDomain;
}

namespace mir {
namespace util {


class Domain : protected BoundingBox {
public:

    // -- Exceptions
    // None

    // -- Contructors

    Domain();
    explicit Domain(const Latitude& north,
                    const Longitude& west,
                    const Latitude& south,
                    const Longitude& east);

    // -- Methods

    /// Contains point
    bool contains(const repres::Iterator::point_ll_t& p) const;

    /// Contains point
    bool contains(const Latitude&, const Longitude&) const;

    using BoundingBox::north;
    using BoundingBox::west;
    using BoundingBox::south;
    using BoundingBox::east;

    // Converter to atlas::RectangularDomain
    operator atlas::RectangularDomain() const;

    /// Check if grid includes the North pole
    bool includesPoleNorth() const;

    /// Check if grid includes the South pole
    bool includesPoleSouth() const;

    /// Check if grid spans the complete range East-West (periodic)
    bool isPeriodicEastWest() const;

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
