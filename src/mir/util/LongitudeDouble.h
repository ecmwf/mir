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


class LongitudeDouble {
public:
    static const LongitudeDouble GLOBE;            // 360
    static const LongitudeDouble DATE_LINE;        // 180
    static const LongitudeDouble MINUS_DATE_LINE;  // -180
    static const LongitudeDouble GREENWICH;        // 0

    // -- Exceptions
    // None

    // -- Constructors

    LongitudeDouble(double value = 0) : value_(value) {}
    LongitudeDouble(const eckit::Fraction& value) : value_(value) {}

    // -- Operators

    bool operator==(double) const;
    bool operator!=(double) const;

    bool operator>(double) const;
    bool operator<(double) const;

    bool operator>=(double) const;
    bool operator<=(double) const;

    LongitudeDouble& operator+=(double value) {
        value_ += value;
        return *this;
    }

    LongitudeDouble& operator-=(double value) {
        value_ -= value;
        return *this;
    }

    LongitudeDouble operator+(double value) const { return {value_ + value}; }

    LongitudeDouble operator-(double value) const { return {value_ - value}; }

    LongitudeDouble operator/(double value) const { return {value_ / value}; }

    LongitudeDouble operator*(double value) const { return {value_ * value}; }

    LongitudeDouble& operator+=(const eckit::Fraction& value) {
        value_ += double(value);
        return *this;
    }

    LongitudeDouble& operator-=(const eckit::Fraction& value) {
        value_ -= double(value);
        return *this;
    }

    LongitudeDouble operator+(const eckit::Fraction& value) const { return {value_ + double(value)}; }

    LongitudeDouble operator-(const eckit::Fraction& value) const { return {value_ - double(value)}; }

    LongitudeDouble& operator+=(const LongitudeDouble& other) {
        value_ += other.value_;
        return *this;
    }

    LongitudeDouble& operator-=(const LongitudeDouble& other) {
        value_ -= other.value_;
        return *this;
    }

    LongitudeDouble operator+(const LongitudeDouble& other) const { return {value_ + other.value_}; }

    LongitudeDouble operator-(const LongitudeDouble& other) const { return {value_ - other.value_}; }

    bool operator==(const LongitudeDouble& other) const { return (*this) == other.value_; }

    bool operator!=(const LongitudeDouble& other) const { return (*this) != other.value_; }

    bool operator>(const LongitudeDouble& other) const { return (*this) > other.value_; }

    bool operator<(const LongitudeDouble& other) const { return (*this) < other.value_; }

    bool operator>=(const LongitudeDouble& other) const { return (*this) >= other.value_; }

    bool operator<=(const LongitudeDouble& other) const { return (*this) <= other.value_; }

    // -- Methods

    void hash(eckit::MD5&) const;

    double value() const { return value_; }

    eckit::Fraction fraction() const { return eckit::Fraction(value_); }

    LongitudeDouble normalise(const LongitudeDouble& minimum) const;

    LongitudeDouble distance(const LongitudeDouble& meridian) const;

protected:
    // -- Methods

    void print(std::ostream&) const;
    void encode(eckit::Stream&) const;
    void decode(eckit::Stream&);

private:
    // -- Members

    double value_;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const LongitudeDouble& x) {
        x.print(s);
        return s;
    }

    friend eckit::Stream& operator<<(eckit::Stream& s, const LongitudeDouble& x) {
        x.encode(s);
        return s;
    }

    friend eckit::Stream& operator>>(eckit::Stream& s, LongitudeDouble& x) {
        x.decode(s);
        return s;
    }

    friend bool operator==(double, const LongitudeDouble&);
    friend bool operator!=(double, const LongitudeDouble&);

    friend bool operator>(double, const LongitudeDouble&);
    friend bool operator<(double, const LongitudeDouble&);

    friend bool operator>=(double, const LongitudeDouble&);

    friend bool operator<=(double, const LongitudeDouble&);

    friend LongitudeDouble operator+(double, const LongitudeDouble&);

    friend LongitudeDouble operator-(double value, const LongitudeDouble& x) { return {value - x.value_}; }
};


}  // namespace mir
