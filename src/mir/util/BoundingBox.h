// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>

#include "mir/util/Types.h"

namespace eckit {
class JSON;
}

struct grib_info;
namespace eckit {
class MD5;
namespace spec {
class Custom;
}  // namespace spec
}  // namespace eckit
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
    // -- Exceptions
    // None

    // -- Constructors

    BoundingBox();
    /// The east normalised to the west, the comparisons to PointLonLat::EPS
    BoundingBox(double north, double west, double south, double east);
    BoundingBox(const param::MIRParametrisation&);
    BoundingBox(const BoundingBox&);

    // -- Destructor

    virtual ~BoundingBox();

    // -- Convertors
    // None

    // -- Operators

    BoundingBox& operator=(const BoundingBox&);

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
    void fillSpec(eckit::spec::Custom&) const;

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

    virtual void print(std::ostream&) const;
    virtual void json(eckit::JSON&) const;

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

    double north_;
    double west_;
    double south_;
    double east_;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

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
