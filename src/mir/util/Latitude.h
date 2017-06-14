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


#ifndef mir_util_Latitude_h
#define mir_util_Latitude_h

#include <iosfwd>
#include "eckit/types/Fraction.h"

namespace eckit {
class MD5;
class Stream;
}

namespace mir {

class Latitude {
public:


    static Latitude NORTH_POLE; // 90
    static Latitude SOUTH_POLE; // -90
    static Latitude EQUATOR; // 0


    // -- Exceptions
    // None

    // -- Contructors
    Latitude(double value = 0): value_(value) {}
    Latitude(const eckit::Fraction& value): value_(value) {}

    double value() const { return value_; }
    eckit::Fraction fraction() const { return eckit::Fraction(value_); }

    bool sameWithGrib1Accuracy(const Latitude& other) const;

    //========================================
    bool operator==(double other) const;
    bool operator!=(double other) const;

    bool operator>(double other) const;
    bool operator<(double other) const;

    bool operator>=(double other) const;
    bool operator<=(double other) const;

    //======================================

    Latitude& operator+=(double value) {
        value_ += value;
        return *this;
    }

    Latitude& operator-=(double value){
        value_ -= value;
        return *this;
    }

    Latitude operator+(double value) const {
        return Latitude(value_ + value);
    }

    Latitude operator-(double value) const {
        return Latitude(value_ - value);
    }

    Latitude operator/(double value) const {
        return Latitude(value_ / value);
    }

 //======================================

    Latitude& operator+=(const eckit::Fraction& value) {
        value_ += double(value);
        return *this;
    }

    Latitude& operator-=(const eckit::Fraction& value){
        value_ -= double(value);
        return *this;
    }

    Latitude operator+(const eckit::Fraction& value) const {
        return Latitude(value_ + double(value));
    }

    Latitude operator-(const eckit::Fraction& value) const {
        return Latitude(value_ - double(value));
    }
    //========================================

    Latitude& operator+=(const Latitude& other) {
        value_ += other.value_;
        return *this;
    }

    Latitude& operator-=(const Latitude& other) {
        value_ -= other.value_;
        return *this;
    }

    Latitude operator+(const Latitude& other) const {
        return Latitude(value_ + other.value_);
    }

    Latitude operator-(const Latitude& other) const{
        return Latitude(value_ - other.value_);
    }

    bool operator==(const Latitude& other) const {
        return (*this) == other.value_;
    }

    bool operator!=(const Latitude& other) const{
        return (*this) != other.value_;
    }

    bool operator>(const Latitude& other) const {
     return (*this) > other.value_;
    }

    bool operator<(const Latitude& other) const  {
        return (*this) < other.value_;
    }

    bool operator>=(const Latitude& other) const{
        return (*this) >= other.value_;
    }

    bool operator<=(const Latitude& other) const{
        return (*this) <= other.value_;
    }

    void hash(eckit::MD5&) const;
    // None

protected:

    // -- Methods

    void print(std::ostream&) const; // Change to virtual if base class
    void encode(eckit::Stream& out) const;
    void decode(eckit::Stream& out);

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

    friend std::ostream&operator<<(std::ostream& s, const Latitude& p) {
        p.print(s);
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

    friend bool operator==(double, const Latitude& other);
    friend bool operator!=(double, const Latitude& other);

    friend bool operator>(double, const Latitude& other);
    friend bool operator<(double, const Latitude& other);

    friend bool operator>=(double, const Latitude& other);
    friend bool operator<=(double, const Latitude& other);

    friend Latitude operator+(double value, const Latitude& l) {
        return Latitude(value + l);
    }

    friend Latitude operator-(double value, const Latitude& l) {
        return Latitude(value - l);
    }

};



}  // namespace mir


#endif

