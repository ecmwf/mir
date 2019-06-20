/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#ifndef mir_util_BoundingBox_h
#define mir_util_BoundingBox_h

#include <iosfwd>

#include "mir/api/Atlas.h"
#include "mir/repres/Iterator.h"
#include "mir/util/Types.h"


struct grib_info;
namespace eckit {
class MD5;
}
namespace mir {
namespace api {
class MIRJob;
}
namespace param {
class MIRParametrisation;
}
namespace util {
class Rotation;
}
}


namespace mir {
namespace util {


class BoundingBox {
public:

    // -- Exceptions
    // None

    // -- Constructors

    BoundingBox();
    BoundingBox(const Latitude& north,
                const Longitude& west,
                const Latitude& south,
                const Longitude& east);
    BoundingBox(const param::MIRParametrisation&);
    BoundingBox(const BoundingBox&);

    // -- Destructor

    virtual ~BoundingBox();

    // -- Convertors
    // None

    // -- Operators

    BoundingBox& operator=(const BoundingBox& other);

    bool operator==(const BoundingBox& other) const;

    bool operator!=(const BoundingBox& other) const {
        return !operator==(other);
    }

    // -- Methods

    // DON'T IMPLEMENT SETTERS

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

    /// Check if covers complete range East-West
    bool isPeriodicWestEast() const;

    /// Contains point
    bool contains(const PointLatLon&) const;

    /// Contains point
    bool contains(const Point2&) const;

    /// Contains point
    bool contains(const Latitude&, const Longitude&) const;

    /// Contains bounding box
    bool contains(const BoundingBox&) const;

    /// Intersects bounding box
    bool intersects(BoundingBox&) const;

    /// Empty
    bool empty() const;

    void fill(grib_info&) const;

    void fill(api::MIRJob&) const;

    void hash(eckit::MD5&) const;

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

    /// Output to stream
    virtual void print(std::ostream&) const;

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

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    /// Output using stream operator
    friend std::ostream &operator<<(std::ostream &s, const BoundingBox &p) {
        p.print(s);
        return s;
    }

};


}  // namespace util
}  // namespace mir


#endif

