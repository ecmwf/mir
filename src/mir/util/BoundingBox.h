/*
 * (C) Copyright 1996-2016 ECMWF.
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
}


namespace mir {
namespace util {

class Increments;

class BoundingBox {
public:

    // -- Exceptions
    // None

    // -- Constructors

    BoundingBox();
    BoundingBox(double north, double west, double south, double east);
    BoundingBox(const param::MIRParametrisation&);
    BoundingBox(const BoundingBox& other);

    // -- Destructor

    ~BoundingBox(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators

    BoundingBox& operator=(const BoundingBox& other) {
        north_ = other.north_;
        west_  = other.west_;
        south_ = other.south_;
        east_  = other.east_;
        return *this;
    }

    bool operator==(const BoundingBox& other) const {
        return (north_ == other.north_) && (south_ == other.south_) && (west_ == other.west_) && (east_ == other.east_);
    }

    bool operator!=(const BoundingBox& other) const {
        return (north_ != other.north_) || (south_ != other.south_) || (west_ != other.west_) || (east_ != other.east_);
    }

    // -- Methods

    // DON'T IMPLEMENT SETTERS

    double north() const {
        return north_;
    }

    double west() const {
        return west_;
    }

    double south() const {
        return south_;
    }

    double east() const {
        return east_;
    }

    bool contains(double lat, double lon) const;

    double normalise(double lon) const;

    void fill(grib_info&) const;

    void fill(api::MIRJob&) const;

    void hash(eckit::MD5&) const;

    size_t computeNi(const util::Increments&) const;
    size_t computeNj(const util::Increments&) const;

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

    // -- Friends
    // None

private:

    // -- Members

    double north_;
    double west_;
    double south_;
    double east_;

    // -- Methods

    void normalise();

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream &operator<<(std::ostream &s, const BoundingBox &p) {
        p.print(s);
        return s;
    }

};


}  // namespace util
}  // namespace mir


#endif

