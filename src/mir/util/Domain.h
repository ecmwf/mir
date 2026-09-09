// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/util/Atlas.h"
#include "mir/util/BoundingBox.h"


namespace mir::util {


class Domain : public BoundingBox {
public:
    // -- Exceptions
    // None

    // -- Constructors

    using BoundingBox::BoundingBox;

    // -- Convertors

    operator atlas::RectangularDomain() const;

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

    void print(std::ostream&) const override;

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


}  // namespace mir::util
