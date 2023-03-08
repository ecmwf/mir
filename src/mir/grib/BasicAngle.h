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


#include <iosfwd>


struct grib_info;

namespace eckit {
class Fraction;
}


namespace mir::grib {


struct Fraction {
    // -- Types

    using value_type = long;

    // -- Exceptions
    // None

    // -- Constructors

    explicit Fraction(double);

    explicit Fraction(const eckit::Fraction&);

    Fraction(value_type _numerator, value_type _denominator) :
        num(_numerator), den(_denominator == 0 || _numerator == 0 ? 1 : _denominator) {}

    // -- Members

    const value_type num;
    const value_type den;
};


Fraction::value_type lcm(Fraction::value_type a, Fraction::value_type b);


template <typename... Longs>
Fraction::value_type lcm(Fraction::value_type a, Fraction::value_type b, Longs... cde) {
    return lcm(a, lcm(b, cde...));
}


long gcd(long a, long b);


template <typename... Longs>
long gcd(long a, long b, Longs... cde) {
    return gcd(a, gcd(b, cde...));
}


struct BasicAngle : Fraction {
    // -- Types
    // None

    // -- Exceptions
    // None

    // -- Constructors

    using Fraction::Fraction;
    BasicAngle(Fraction, Fraction, Fraction, Fraction, Fraction, Fraction);
    explicit BasicAngle(const grib_info&);

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    void fillGrib(grib_info&) const;
    Fraction::value_type numerator(const Fraction&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods

    static void list(std::ostream&);
};


}  // namespace mir::grib
