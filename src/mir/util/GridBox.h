/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_util_GridBox_h
#define mir_util_GridBox_h

#include <iosfwd>


namespace mir {
namespace util {


class GridBox {
public:
    // -- Types

    struct LongitudeRange {
        LongitudeRange(double west, double east);
        bool intersects(const LongitudeRange&) const;
        static double normalise(double lon, double minimum);
        double west;
        double east;
        static constexpr double GLOBE = 360.;
    };

    struct LatitudeRange {
        LatitudeRange(double south, double north);
        bool intersects(const LatitudeRange&) const;
        double south;
        double north;
    };

    // -- Exceptions
    // None

    // -- Constructors

    GridBox(LatitudeRange&, LongitudeRange&);

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    double area() const;

    static double normalise(double lon, double minimum);

    bool intersects(GridBox&) const;

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
    // -- Members

    double north_;
    double west_;
    double south_;
    double east_;
    static constexpr double GLOBE = 360.;

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


}  // namespace util
}  // namespace mir


#endif
