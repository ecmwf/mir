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


#ifndef mir_util_LongitudeFraction_h
#define mir_util_LongitudeFraction_h

#include <iosfwd>
#include "eckit/types/Fraction.h"

namespace eckit {
class MD5;
class Stream;
}


namespace mir {


class LongitudeFraction {
public:

    static LongitudeFraction GLOBE; // 360
    static LongitudeFraction DATE_LINE; // 180
    static LongitudeFraction MINUS_DATE_LINE; // -180
    static LongitudeFraction GREENWICH; // 0

    // -- Exceptions
    // None

    // -- Contructors

    LongitudeFraction(double value = 0): value_(value) {}
    LongitudeFraction(const eckit::Fraction& value): value_(value) {}

    // -- Operators

    bool operator==(double other) const;
    bool operator!=(double other) const;

    bool operator>(double other) const;
    bool operator<(double other) const;

    bool operator>=(double other) const;
    bool operator<=(double other) const;

    bool operator==(const eckit::Fraction& other) const;
    bool operator!=(const eckit::Fraction& other) const;

    bool operator>(const eckit::Fraction& other) const;
    bool operator<(const eckit::Fraction& other) const;

    bool operator>=(const eckit::Fraction& other) const;
    bool operator<=(const eckit::Fraction& other) const;

    //======================================

    LongitudeFraction& operator+=(double value) {
        value_ += value;
        return *this;
    }

    LongitudeFraction& operator-=(double value){
        value_ -= value;
        return *this;
    }

    LongitudeFraction operator+(double value) const {
        return LongitudeFraction(value_ + value);
    }

    LongitudeFraction operator-(double value) const {
        return LongitudeFraction(value_ - value);
    }

    LongitudeFraction operator/(double value) const {
        return LongitudeFraction(value_ / value);
    }

    LongitudeFraction operator*(double value) const {
        return LongitudeFraction(value_ * value);
    }

    //======================================

    LongitudeFraction& operator+=(const eckit::Fraction& value) {
        value_ += value;
        return *this;
    }

    LongitudeFraction& operator-=(const eckit::Fraction& value){
        value_ -= value;
        return *this;
    }

    LongitudeFraction operator+(const eckit::Fraction& value) const {
        return LongitudeFraction(value_ + value);
    }

    LongitudeFraction operator-(const eckit::Fraction& value) const {
        return LongitudeFraction(value_ - value);
    }

    LongitudeFraction operator/(const eckit::Fraction& value) const {
        return LongitudeFraction(value_ / value);
    }

    LongitudeFraction operator*(const eckit::Fraction& value) const {
        return LongitudeFraction(value_ * value);
    }

    //========================================

    LongitudeFraction& operator+=(const LongitudeFraction& other) {
        value_ += other.value_;
        return *this;
    }

    LongitudeFraction& operator-=(const LongitudeFraction& other) {
        value_ -= other.value_;
        return *this;
    }

    LongitudeFraction operator+(const LongitudeFraction& other) const {
        return LongitudeFraction(value_ + other.value_);
    }

    LongitudeFraction operator-(const LongitudeFraction& other) const{
        return LongitudeFraction(value_ - other.value_);
    }

    bool operator==(const LongitudeFraction& other) const;

    bool operator!=(const LongitudeFraction& other) const;

    bool operator>(const LongitudeFraction& other) const;

    bool operator<(const LongitudeFraction& other) const;

    bool operator>=(const LongitudeFraction& other) const;

    bool operator<=(const LongitudeFraction& other) const;

    // -- Methods

    void hash(eckit::MD5&) const;

    double value() const { return value_; }

    eckit::Fraction fraction() const { return eckit::Fraction(value_); }

    bool sameWithGrib1Accuracy(const LongitudeFraction& other) const;

    LongitudeFraction normalise(const LongitudeFraction& minimum) const;

protected:

    // -- Methods

    void print(std::ostream&) const; // Change to virtual if base class
    void encode(eckit::Stream& out) const;
    void decode(eckit::Stream& out);

private:

    // -- Members

    eckit::Fraction value_;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream&operator<<(std::ostream& s, const LongitudeFraction& p) {
        p.print(s);
        return s;
    }

    friend eckit::Stream& operator<<(eckit::Stream& s, const LongitudeFraction& x) {
        x.encode(s);
        return s;
    }

    friend eckit::Stream& operator>>(eckit::Stream& s, LongitudeFraction& x) {
        x.decode(s);
        return s;
    }

    friend bool operator==(double, const LongitudeFraction& other);
    friend bool operator!=(double, const LongitudeFraction& other);

    friend bool operator>(double, const LongitudeFraction& other);
    friend bool operator<(double, const LongitudeFraction& other);

    friend bool operator>=(double, const LongitudeFraction& other);

    friend bool operator<=(double value, const LongitudeFraction& other) {
        return value <= other.value();
    }

    friend LongitudeFraction operator+(double, const LongitudeFraction& other);

    friend LongitudeFraction operator-(double value, const LongitudeFraction& l) {
        return LongitudeFraction(value - l.value_);
    }

};


}  // namespace mir


#endif

