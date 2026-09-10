// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>

#include "mir/util/Types.h"


namespace mir::util {


class GridBox {
public:
    // -- Exceptions
    // None

    // -- Constructors

    GridBox(double north, double west, double south, double east);
    GridBox() = default;

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    double area() const;
    double diagonal() const;
    Point2 centre() const;

    bool contains(const Point2&) const;
    bool intersects(GridBox&) const;

    double north() const { return north_; }
    double west() const { return west_; }
    double south() const { return south_; }
    double east() const { return east_; }

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

    // -- Friends
    // None

private:
    // -- Members

    double north_;
    double west_;
    double south_;
    double east_;

    // -- Methods

    void print(std::ostream&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const GridBox& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::util
