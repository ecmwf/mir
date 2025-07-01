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
}

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
}  // namespace mir


namespace mir::util {


class BoundingBox {
public:
    // -- Constructors

    BoundingBox();
    BoundingBox(double north, double west, double south, double east);
    BoundingBox(const param::MIRParametrisation&);

    BoundingBox(const BoundingBox&) = default;
    BoundingBox(BoundingBox&&)      = default;

    // -- Destructor

    virtual ~BoundingBox() = default;

    // -- Operators

    BoundingBox& operator=(const BoundingBox&) = default;
    BoundingBox& operator=(BoundingBox&&)      = default;

    bool operator==(const BoundingBox&) const;

    bool operator!=(const BoundingBox& other) const { return !operator==(other); }

    // -- Methods

    // DON'T IMPLEMENT SETTERS

    double north() const { return north_; }
    double west() const { return west_; }
    double south() const { return south_; }
    double east() const { return east_; }

    bool isPeriodicWestEast() const;

    bool contains(const PointLonLat&) const;
    bool contains(double lat, double lon) const;
    bool contains(const BoundingBox&) const;

    bool intersects(BoundingBox&) const;

    bool empty() const;

    void fillGrib(grib_info&) const;

    void fillJob(api::MIRJob&) const;

    void hash(eckit::MD5&) const;

    void makeName(std::ostream&) const;

protected:
    // -- Methods

    virtual void print(std::ostream&) const;
    virtual void json(eckit::JSON&) const;

private:
    // -- Members

    double north_;
    double west_;
    double south_;
    double east_;

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const BoundingBox& p) {
        p.print(s);
        return s;
    }

    friend eckit::JSON& operator<<(eckit::JSON& s, const BoundingBox& p) {
        p.json(s);
        return s;
    }
};


}  // namespace mir::util
