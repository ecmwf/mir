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


#ifndef RotatedIterator_H
#define RotatedIterator_H

#include "mir/repres/Iterator.h"
#include "eckit/memory/ScopedPtr.h"
#include "mir/util/Rotation.h"
#include "eckit/geometry/RotateGrid.h"

namespace mir {
namespace util {


class RotatedIterator : public repres::Iterator {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    RotatedIterator(Iterator* iterator, const util::Rotation &rotation);

    // -- Destructor

    virtual ~RotatedIterator(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  protected:

    // -- Members

    // -- Methods


    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // No copy allowed

    RotatedIterator(const RotatedIterator &);
    RotatedIterator &operator=(const RotatedIterator &);

    // -- Members

    eckit::ScopedPtr<Iterator> iterator_;
    util::Rotation rotation_;
    eckit::geometry::RotateGrid rotate_;

    // -- Methods
    // None

    // -- Overridden methods

    virtual bool next(double &lat, double &lon);
    virtual void print(std::ostream &) const; // Change to virtual if base class

    // -- Class members

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream &operator<<(std::ostream &s, const RotatedIterator &p) {
        p.print(s);
        return s;
    }

};


}  // namespace repres
}  // namespace mir
#endif

