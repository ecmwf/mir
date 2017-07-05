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


#ifndef mir_util_Increments_h
#define mir_util_Increments_h

#include <iosfwd>
#include "eckit/types/Fraction.h"


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
class BoundingBox;
class Shift;
}
}


namespace mir {
namespace util {


class Increments {
public:

    // -- Exceptions
    // None

    // -- Contructors

    Increments(const param::MIRParametrisation&);
    Increments(const Increments&);

    // NOTE: maybe to be substituded by (Longitude&, Latitude&), and no defaults
    explicit Increments(double west_east = 0, double south_north = 0);
    explicit Increments(const eckit::Fraction& west_east, const eckit::Fraction& south_north);
    explicit Increments(double west_east, const eckit::Fraction& south_north);
    explicit Increments(const eckit::Fraction& west_east, double south_north);

    // -- Destructor

    ~Increments(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators

    bool operator==(const Increments& other) const {
        return (west_east_ == other.west_east_) && (south_north_ == other.south_north_);
    }

    bool operator!=(const Increments& other) const {
        return (west_east_ != other.west_east_) || (south_north_ != other.south_north_);
    }

    // -- Methods

    // Retrurn true is this is a multiple of other, e.g. 2/2 is a multiple of 1/1
    bool multipleOf(const Increments& other) const;

    void ratio(const Increments& other, size_t& we, size_t& ns) const;

    bool matches(const BoundingBox&) const;

    Increments bestSubsetting(const BoundingBox&) const;

    Shift shiftFromZeroZero(const BoundingBox&) const;

    BoundingBox globalBoundingBox(const Shift&) const;

    size_t computeNi(const BoundingBox&) const;

    size_t computeNj(const BoundingBox&) const;

    const eckit::Fraction& west_east() const {
        return west_east_;
    }

    const eckit::Fraction& south_north() const {
        return south_north_;
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

    eckit::Fraction west_east_;
    eckit::Fraction south_north_;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream &operator<<(std::ostream& s, const Increments& p) {
        p.print(s);
        return s;
    }

};


}  // namespace util
}  // namespace mir


#endif

