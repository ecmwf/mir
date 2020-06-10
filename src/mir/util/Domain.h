/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_util_Domain_h
#define mir_util_Domain_h

#include "mir/util/BoundingBox.h"
#include "mir/util/RectangularDomain.h"


namespace mir {
namespace util {


class Domain : public BoundingBox {
public:
    // -- Exceptions
    // None

    // -- Constructors

    using BoundingBox::BoundingBox;
    Domain(const BoundingBox& bbox) : BoundingBox(bbox) {}

    // -- Convertors

    operator RectangularDomain() const;

    // -- Operators

    using BoundingBox::operator=;
    using BoundingBox::operator==;
    using BoundingBox::operator!=;

    // -- Methods
    // None

    bool includesPoleNorth() const;

    bool includesPoleSouth() const;

    bool isGlobal() const { return includesPoleNorth() && includesPoleSouth() && isPeriodicWestEast(); }

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

    virtual void print(std::ostream&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None

private:
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

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const Domain& p) {
        p.print(s);
        return s;
    }
};


}  // namespace util
}  // namespace mir


#endif
