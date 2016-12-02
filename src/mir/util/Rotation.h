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


#ifndef Rotation_H
#define Rotation_H


#include <iosfwd>

struct grib_info;

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

class Rotation {
public:

    // -- Exceptions
    // None

    // -- Contructors

    explicit Rotation(const param::MIRParametrisation &);
    explicit Rotation(double south_pole_latitude = 0,
                      double south_pole_longitude = 0,
                      double south_pole_rotation_angle = 0);

    // -- Destructor

    ~Rotation(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators

    bool operator==(const Rotation& other) const;

    // bool operator!=(const Rotation& other) const {
    //     return (west_east_ != other.west_east_) || (south_north_ != other.south_north_);
    // }

    // // -- Methods

    double south_pole_latitude() const {
        return south_pole_latitude_;
    }

    double south_pole_longitude() const {
        return south_pole_longitude_;
    }

    double south_pole_rotation_angle() const {
        return south_pole_rotation_angle_;
    }


    //
    void fill(grib_info &) const;
    void fill(api::MIRJob &) const;



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

    void print(std::ostream &) const; // Change to virtual if base class

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // No copy allowed


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

    friend std::ostream &operator<<(std::ostream &s, const Rotation &p) {
        p.print(s);
        return s;
    }

};


}  // namespace util
}  // namespace mir
#endif

