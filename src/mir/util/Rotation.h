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

#include "eckit/geo/projection/Rotation.h"

#include "mir/util/Types.h"


struct grib_info;

namespace mir {
namespace api {
class MIRJob;
}
namespace param {
class MIRParametrisation;
}
namespace util {
class BoundingBox;
}
}  // namespace mir


namespace mir::util {


/**
 * @brief WMO specification on rotated grids
 *
 * (6) Three parameters define a general latitude/longitude coordinate system, formed by a general rotation
 * of the sphere. One
 *     choice for these parameters is:
 *     (a) The geographic latitude in degrees of the southern pole of the coordinate system, θp for example;
 *
 *     (b) The geographic longitude in degrees of the southern pole of the coordinate system, λp for example;
 *
 *     (c) The angle of rotation in degrees about the new polar axis (measured clockwise when looking from
 *         the southern to the northern pole) of the coordinate system, assuming the new axis to have been
 *         obtained by first rotating the sphere through λp degrees about the geographic polar axis, and then
 *         rotating through (90 + θp) degrees so that the southern pole moved along the (previously rotated)
 *         Greenwich meridian.
 */
class Rotation {
public:
    // -- Constructors
    explicit Rotation(const param::MIRParametrisation&);
    explicit Rotation(const Latitude& south_pole_latitude   = Latitude::SOUTH_POLE,
                      const Longitude& south_pole_longitude = Longitude::GREENWICH,
                      double south_pole_rotation_angle      = 0.);

    // -- Destructor

    ~Rotation() = default;


    // -- Operators

    bool operator==(const Rotation&) const;

    // -- Methods

    const eckit::geo::projection::Rotation& rotation() const { return rotation_; }
    BoundingBox boundingBox(const BoundingBox&) const;

    Latitude south_pole_latitude() const { return rotation_.south_pole().lat; }
    Longitude south_pole_longitude() const { return rotation_.south_pole().lon; }
    double south_pole_rotation_angle() const { return rotation_.angle(); }

    void fillGrib(grib_info&) const;
    void fillJob(api::MIRJob&) const;
    void makeName(std::ostream&) const;

private:
    // -- Members

    eckit::geo::projection::Rotation rotation_;

    // -- Methods

    void print(std::ostream&) const;

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const Rotation& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::util
