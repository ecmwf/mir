/*
 * (C) Copyright 1996-2015 ECMWF.
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


#ifndef mir_util_Rotation_h
#define mir_util_Rotation_h

#include <iosfwd>


struct grib_info;

namespace atlas {
class Grid;
}
namespace mir {
namespace param {
class MIRParametrisation;
}
namespace api {
class MIRJob;
}
}


namespace mir {
namespace util {


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

    // -- Exceptions
    // None

    // -- Contructors
    explicit Rotation(const param::MIRParametrisation&);
    explicit Rotation(double south_pole_latitude = -90,
                      double south_pole_longitude = 0,
                      double south_pole_rotation_angle = 0);

    // -- Destructor
    ~Rotation();  // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators

    bool operator==(const Rotation& other) const;

    // -- Methods

    atlas::Grid rotate(const atlas::Grid&) const;

    double south_pole_latitude() const {
        return south_pole_latitude_;
    }

    double south_pole_longitude() const {
        return south_pole_longitude_;
    }

    double south_pole_rotation_angle() const {
        return south_pole_rotation_angle_;
    }

    void fill(grib_info&) const;
    void fill(api::MIRJob&) const;
    void makeName(std::ostream& out) const;

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

    void print(std::ostream&) const; // Change to virtual if base class

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Members

    double south_pole_latitude_;
    double south_pole_longitude_;
    double south_pole_rotation_angle_;

    // -- Methods

    void normalize();

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream&operator<<(std::ostream& s, const Rotation& p) {
        p.print(s);
        return s;
    }

};


}  // namespace util
}  // namespace mir


#endif

