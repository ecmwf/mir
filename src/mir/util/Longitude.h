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


#ifndef mir_util_Longitude_h
#define mir_util_Longitude_h

#include <iosfwd>
#include "eckit/types/Fraction.h"

namespace eckit {
class MD5;
class Stream;
}

namespace mir {

class Longitude {
public:

    static Longitude GLOBE; // 360
    static Longitude DATE_LINE; // 180
    static Longitude MINUS_DATE_LINE; // -180
    static Longitude GREENWICH; // 0

    // -- Exceptions
    // None

    // -- Contructors
    Longitude(double value = 0): value_(value) {}
    Longitude(const eckit::Fraction& value): value_(value) {}

    double value() const { return value_; }
    eckit::Fraction fraction() const { return eckit::Fraction(value_); }

    bool sameWithGrib1Accuracy(const Longitude& other) const;
    //========================================
    bool operator==(double other) const;
    bool operator!=(double other) const;

    bool operator>(double other) const;
    bool operator<(double other) const;

    bool operator>=(double other) const;
    bool operator<=(double other) const;

    //======================================

    Longitude& operator+=(double value) {
        value_ += value;
        return *this;
    }

    Longitude& operator-=(double value){
        value_ -= value;
        return *this;
    }

    Longitude operator+(double value) const {
        return Longitude(value_ + value);
    }

    Longitude operator-(double value) const {
        return Longitude(value_ - value);
    }

    Longitude operator/(double value) const {
        return Longitude(value_ / value);
    }

    Longitude operator*(double value) const {
        return Longitude(value_ * value);
    }

    //======================================

    Longitude& operator+=(const eckit::Fraction& value) {
        value_ += double(value);
        return *this;
    }

    Longitude& operator-=(const eckit::Fraction& value){
        value_ -= double(value);
        return *this;
    }

    Longitude operator+(const eckit::Fraction& value) const {
        return Longitude(value_ + double(value));
    }

    Longitude operator-(const eckit::Fraction& value) const {
        return Longitude(value_ - double(value));
    }
    //========================================
    //========================================

    Longitude& operator+=(const Longitude& other) {
        value_ += other.value_;
        return *this;
    }

    Longitude& operator-=(const Longitude& other) {
        value_ -= other.value_;
        return *this;
    }

    Longitude operator+(const Longitude& other) const {
        return Longitude(value_ + other.value_);
    }

    Longitude operator-(const Longitude& other) const{
        return Longitude(value_ - other.value_);
    }

    bool operator==(const Longitude& other) const {
        return (*this) == other.value_;
    }

    bool operator!=(const Longitude& other) const{
        return (*this) != other.value_;
    }

    bool operator>(const Longitude& other) const {
     return (*this) > other.value_;
    }

    bool operator<(const Longitude& other) const  {
        return (*this) < other.value_;
    }

    bool operator>=(const Longitude& other) const{
        return (*this) >= other.value_;
    }

    bool operator<=(const Longitude& other) const{
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

    friend std::ostream&operator<<(std::ostream& s, const Longitude& p) {
        p.print(s);
        return s;
    }

    friend eckit::Stream& operator<<(eckit::Stream& s, const Longitude& x) {
        x.encode(s);
        return s;
    }

    friend eckit::Stream& operator>>(eckit::Stream& s, Longitude& x) {
        x.decode(s);
        return s;
    }

    friend bool operator==(double, const Longitude& other);
    friend bool operator!=(double, const Longitude& other);

    friend bool operator>(double, const Longitude& other);
    friend bool operator<(double, const Longitude& other);

    friend bool operator>=(double, const Longitude& other);
    friend bool operator<=(double, const Longitude& other);

    friend Longitude operator+(double, const Longitude& other);

    friend Longitude operator-(double value, const Longitude& l) {
        return Longitude(value - l);
    }

};



}  // namespace mir


#endif

