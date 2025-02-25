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

namespace atlas {
class Grid;
class Projection;
}  // namespace atlas
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
 * === end WMO specification ===
 *
 * gribs use the following convention: (from Shahram)
 *
 * Horizontally:  Points scan in the +i (+x) direction
 * Vertically:    Points scan in the -j (-y) direction
 *
 * The way I verified this was to look at our SAMPLE files (which IFS uses). I also verified that IFS does
 * not modify the scanning modes so whatever the samples say, is the convention
 */
class Rotation {
public:
    // -- Constructors

    explicit Rotation(const param::MIRParametrisation&);
    explicit Rotation(PointLonLat south_pole = {Longitude::GREENWICH.value(), Latitude::SOUTH_POLE.value()},
                      double angle           = 0);

    Rotation(double south_pole_lat, double south_pole_lon) : Rotation(PointLonLat{south_pole_lon, south_pole_lat}) {}

    // -- Operators

    bool operator==(const Rotation&) const;

    // -- Methods

    PointLonLat rotate(PointLonLat) const;
    atlas::Grid rotate(const atlas::Grid&) const;

    BoundingBox boundingBox(const BoundingBox&) const;

    PointLonLat southPole() const { return rotation_.south_pole(); }
    double angle() const { return rotation_.angle(); }
    bool rotated() const { return rotation_.rotated(); }

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
