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


#ifndef Bitmap_H
#define Bitmap_H


#include "eckit/filesystem/PathName.h"


namespace mir {
namespace util {


class Bitmap {
  public:

// -- Exceptions
    // None

// -- Contructors

    Bitmap(const eckit::PathName&);

// -- Destructor

    ~Bitmap(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods

    const size_t width() const {
        return width_;
    }
    const size_t height() const {
        return height_;
    }

    bool on(size_t i, size_t j) const {
        return bitmap_[i][j];
    }

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

// No copy allowed

    Bitmap(const Bitmap&);
    Bitmap& operator=(const Bitmap&);

// -- Members

    eckit::PathName path_;
    std::vector<std::vector<bool> > bitmap_;
    size_t width_;
    size_t height_;

// -- Methods
    // None

// -- Overridden methods


// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    friend std::ostream& operator<<(std::ostream& s,const Bitmap& p) {
        p.print(s);
        return s;
    }

};


}  // namespace util
}  // namespace mir
#endif

