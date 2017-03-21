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


#ifndef Increments_H
#define Increments_H


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

class BoundingBox;

class Increments {
public:

    // -- Exceptions
    // None

    // -- Contructors

    explicit Increments(const param::MIRParametrisation &);
    explicit Increments(double west_east = 0, double south_north = 0);

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

    // Retrurn true is this is a multiple of other
    // e.g. 2/2 is a multiple of 1/1
    bool multipleOf(const Increments& other) const;
    void ratio(const Increments& other, size_t& we, size_t& ns) const;

    bool matches(const BoundingBox& bbox) const;
    Increments bestSubsetting(const BoundingBox& bbox) const;

    double west_east() const {
        return west_east_;
    }

    double south_north() const {
        return south_north_;
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

    double west_east_;
    double south_north_;

    // -- Methods

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream &operator<<(std::ostream &s, const Increments &p) {
        p.print(s);
        return s;
    }

};


}  // namespace util
}  // namespace mir
#endif

