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


#ifndef ShiftIterator_H
#define ShiftIterator_H

#include "eckit/memory/ScopedPtr.h"
#include "mir/repres/Iterator.h"
// #include "mir/util/RotateGrid.h"
#include "mir/util/Shift.h"


namespace mir {
namespace util {


class ShiftIterator : public repres::Iterator {
public:

    // -- Exceptions
    // None

    // -- Contructors

    ShiftIterator(Iterator* iterator, const Shift& shift);

    // -- Destructor

    virtual ~ShiftIterator(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

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
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // No copy allowed

    ShiftIterator(const ShiftIterator&);
    ShiftIterator& operator=(const ShiftIterator&);

    // -- Members

    eckit::ScopedPtr<Iterator> iterator_;
    Shift shift_;

    // -- Methods
    // None

    // -- Overridden methods

    virtual bool next(double& lat, double& lon);

    virtual void print(std::ostream&) const; // Change to virtual if base class

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream &operator<<(std::ostream& s, const ShiftIterator& p) {
        p.print(s);
        return s;
    }

};


}  // namespace util
}  // namespace mir


#endif

