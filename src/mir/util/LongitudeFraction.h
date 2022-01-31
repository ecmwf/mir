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


#pragma once

#include <iosfwd>

#include "eckit/types/Fraction.h"


namespace eckit {
class MD5;
class Stream;
}  // namespace eckit


namespace mir {


class LongitudeFraction {
public:
    static const LongitudeFraction GLOBE;            // 360
    static const LongitudeFraction DATE_LINE;        // 180
    static const LongitudeFraction MINUS_DATE_LINE;  // -180
    static const LongitudeFraction GREENWICH;        // 0

    // -- Exceptions
    // None

    // -- Constructors

    LongitudeFraction(double value = 0) : value_(value) {}
    LongitudeFraction(const eckit::Fraction& value) : value_(value) {}

    // -- Operators

    bool operator==(double) const;
    bool operator!=(double) const;

    bool operator>(double) const;
    bool operator<(double) const;

    bool operator>=(double) const;
    bool operator<=(double) const;

    bool operator==(const eckit::Fraction&) const;
    bool operator!=(const eckit::Fraction&) const;

    bool operator>(const eckit::Fraction&) const;
    bool operator<(const eckit::Fraction&) const;

    bool operator>=(const eckit::Fraction&) const;
    bool operator<=(const eckit::Fraction&) const;

    LongitudeFraction& operator+=(double value) {
        value_ += value;
        return *this;
    }

    LongitudeFraction& operator-=(double value) {
        value_ -= value;
        return *this;
    }

    LongitudeFraction operator+(double value) const { return {value_ + value}; }

    LongitudeFraction operator-(double value) const { return {value_ - value}; }

    LongitudeFraction operator/(double value) const { return {value_ / value}; }

    LongitudeFraction operator*(double value) const { return {value_ * value}; }

    LongitudeFraction& operator+=(const eckit::Fraction& value) {
        value_ += value;
        return *this;
    }

    LongitudeFraction& operator-=(const eckit::Fraction& value) {
        value_ -= value;
        return *this;
    }

    LongitudeFraction operator+(const eckit::Fraction& value) const { return {value_ + value}; }

    LongitudeFraction operator-(const eckit::Fraction& value) const { return {value_ - value}; }

    LongitudeFraction operator/(const eckit::Fraction& value) const { return {value_ / value}; }

    LongitudeFraction operator*(const eckit::Fraction& value) const { return {value_ * value}; }

    LongitudeFraction& operator+=(const LongitudeFraction& other) {
        value_ += other.value_;
        return *this;
    }

    LongitudeFraction& operator-=(const LongitudeFraction& other) {
        value_ -= other.value_;
        return *this;
    }

    LongitudeFraction operator+(const LongitudeFraction& other) const { return {value_ + other.value_}; }

    LongitudeFraction operator-(const LongitudeFraction& other) const { return {value_ - other.value_}; }

    bool operator==(const LongitudeFraction&) const;
    bool operator!=(const LongitudeFraction&) const;

    bool operator>(const LongitudeFraction&) const;
    bool operator<(const LongitudeFraction&) const;

    bool operator>=(const LongitudeFraction&) const;
    bool operator<=(const LongitudeFraction&) const;

    // -- Methods

    void hash(eckit::MD5&) const;

    double value() const { return value_; }

    eckit::Fraction fraction() const { return eckit::Fraction(value_); }

    LongitudeFraction normalise(const LongitudeFraction& minimum) const;

    LongitudeFraction distance(const LongitudeFraction& meridian) const;

protected:
    // -- Methods

    void print(std::ostream&) const;
    void encode(eckit::Stream&) const;
    void decode(eckit::Stream&);

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

    friend std::ostream& operator<<(std::ostream& s, const LongitudeFraction& x) {
        x.print(s);
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

    friend bool operator==(double, const LongitudeFraction&);
    friend bool operator!=(double, const LongitudeFraction&);

    friend bool operator>(double, const LongitudeFraction&);
    friend bool operator<(double, const LongitudeFraction&);

    friend bool operator>=(double, const LongitudeFraction&);

    friend bool operator<=(double value, const LongitudeFraction& x) { return value <= x.value(); }

    friend LongitudeFraction operator+(double, const LongitudeFraction&);

    friend LongitudeFraction operator-(double value, const LongitudeFraction& x) { return {value - x.value_}; }
};


}  // namespace mir
