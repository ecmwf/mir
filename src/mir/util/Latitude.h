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


class Latitude {
public:
    static const Latitude GLOBE;       // 180
    static const Latitude NORTH_POLE;  // 90
    static const Latitude SOUTH_POLE;  // -90
    static const Latitude EQUATOR;     // 0

    // -- Exceptions
    // None

    // -- Constructors
    Latitude(double value = 0) : value_(value) {}
    Latitude(const eckit::Fraction& value) : value_(value) {}

    double value() const { return value_; }
    eckit::Fraction fraction() const { return eckit::Fraction(value_); }

    // -- Operators

    bool operator==(double) const;
    bool operator!=(double) const;

    bool operator>(double) const;
    bool operator<(double) const;

    bool operator>=(double) const;
    bool operator<=(double) const;

    Latitude& operator+=(double value) {
        value_ += value;
        return *this;
    }

    Latitude& operator-=(double value) {
        value_ -= value;
        return *this;
    }

    Latitude operator+(double value) const { return {value_ + value}; }

    Latitude operator-(double value) const { return {value_ - value}; }

    Latitude operator/(double value) const { return {value_ / value}; }

    Latitude& operator+=(const eckit::Fraction& value) {
        value_ += double(value);
        return *this;
    }

    Latitude& operator-=(const eckit::Fraction& value) {
        value_ -= double(value);
        return *this;
    }

    Latitude operator+(const eckit::Fraction& value) const { return {value_ + double(value)}; }

    Latitude operator-(const eckit::Fraction& value) const { return {value_ - double(value)}; }

    Latitude& operator+=(const Latitude& other) {
        value_ += other.value_;
        return *this;
    }

    Latitude& operator-=(const Latitude& other) {
        value_ -= other.value_;
        return *this;
    }

    Latitude operator+(const Latitude& other) const { return {value_ + other.value_}; }

    Latitude operator-(const Latitude& other) const { return {value_ - other.value_}; }

    bool operator==(const Latitude& other) const { return (*this) == other.value_; }

    bool operator!=(const Latitude& other) const { return (*this) != other.value_; }

    bool operator>(const Latitude& other) const { return (*this) > other.value_; }

    bool operator<(const Latitude& other) const { return (*this) < other.value_; }

    bool operator>=(const Latitude& other) const { return (*this) >= other.value_; }

    bool operator<=(const Latitude& other) const { return (*this) <= other.value_; }

    // -- Methods

    void hash(eckit::MD5&) const;

    Latitude distance(const Latitude& parallel) const;

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

    friend std::ostream& operator<<(std::ostream& s, const Latitude& x) {
        x.print(s);
        return s;
    }

    friend eckit::Stream& operator<<(eckit::Stream& s, const Latitude& x) {
        x.encode(s);
        return s;
    }

    friend eckit::Stream& operator>>(eckit::Stream& s, Latitude& x) {
        x.decode(s);
        return s;
    }

    friend bool operator==(double, const Latitude&);
    friend bool operator!=(double, const Latitude&);

    friend bool operator>(double, const Latitude&);
    friend bool operator<(double value, const Latitude& x) { return value < x.value(); }

    friend bool operator>=(double, const Latitude&);
    friend bool operator<=(double, const Latitude&);

    friend Latitude operator+(double value, const Latitude& x) { return {value + x.value_}; }

    friend Latitude operator-(double value, const Latitude& x) { return {value - x.value_}; }
};


}  // namespace mir
