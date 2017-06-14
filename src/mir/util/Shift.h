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


#ifndef Shift_H
#define Shift_H


#include <iosfwd>
#include "eckit/types/Fraction.h"


namespace mir {

namespace api {
class MIRJob;
}
}

namespace mir {
namespace util {


class Shift {
public:

    // -- Exceptions
    // None

    // -- Contructors

    explicit Shift(const eckit::Fraction& west_east = eckit::Fraction(),
        const eckit::Fraction& south_north = eckit::Fraction());

    // -- Destructor

    ~Shift(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators

    bool operator==(const Shift& other) const {
        return (west_east_ == other.west_east_) && (south_north_ == other.south_north_);
    }

    bool operator!=(const Shift& other) const {
        return (west_east_ != other.west_east_) || (south_north_ != other.south_north_);
    }

    operator bool() const {
        return west_east_ != 0 || south_north_ != 0;
    }

    // -- Methods

    const eckit::Fraction& west_east() const {
        return west_east_;
    }

    const eckit::Fraction& south_north() const {
        return south_north_;
    }

    void fill(api::MIRJob &) const;
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

    eckit::Fraction west_east_;
    eckit::Fraction south_north_;

    // -- Methods

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream &operator<<(std::ostream &s, const Shift &p) {
        p.print(s);
        return s;
    }

};


}  // namespace util
}  // namespace mir
#endif

