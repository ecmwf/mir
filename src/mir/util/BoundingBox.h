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


#ifndef BoundingBox_H
#define BoundingBox_H


#include <iosfwd>

struct grib_info;

namespace mir {
namespace param {
class MIRParametrisation;
}
}

namespace mir {
namespace util {

class BoundingBox {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    BoundingBox(const param::MIRParametrisation &);
    BoundingBox(double north=90, double west=0, double south=-90, double east=360);

    // -- Destructor

    ~BoundingBox(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators

    bool operator==(const BoundingBox& other) const {
        return (north_ == other.north_) && (south_ == other.south_) && (west_ == other.west_) && (east_ == other.east_);
    }

    bool operator!=(const BoundingBox& other) const {
        return (north_ != other.north_) || (south_ != other.south_) || (west_ != other.west_) || (east_ != other.east_);
    }

    bool contains(double lat, double lon) const;
    double normalise(double lon) const;

    // -- Methods

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

    //
    void fill(grib_info &) const;
    bool global() const;



    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    static bool greater_equal(double a, double b);

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

