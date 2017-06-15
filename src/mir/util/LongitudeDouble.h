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


#ifndef mir_util_LongitudeDouble_h
#define mir_util_LongitudeDouble_h

#include <iosfwd>
#include "eckit/types/Fraction.h"

namespace eckit {
class MD5;
class Stream;
}

namespace mir {

class LongitudeDouble {
public:

    static LongitudeDouble GLOBE; // 360
    static LongitudeDouble DATE_LINE; // 180
    static LongitudeDouble MINUS_DATE_LINE; // -180
    static LongitudeDouble GREENWICH; // 0

    // -- Exceptions
    // None

    // -- Contructors
    LongitudeDouble(double value = 0): value_(value) {}
    LongitudeDouble(const eckit::Fraction& value): value_(value) {}

    double value() const { return value_; }
    eckit::Fraction fraction() const { return eckit::Fraction(value_); }

    bool sameWithGrib1Accuracy(const LongitudeDouble& other) const;
    //========================================
    bool operator==(double other) const;
    bool operator!=(double other) const;

    bool operator>(double other) const;
    bool operator<(double other) const;

    bool operator>=(double other) const;
    bool operator<=(double other) const;

    //======================================

    LongitudeDouble& operator+=(double value) {
        value_ += value;
        return *this;
    }

    LongitudeDouble& operator-=(double value){
        value_ -= value;
        return *this;
    }

    LongitudeDouble operator+(double value) const {
        return LongitudeDouble(value_ + value);
    }

    LongitudeDouble operator-(double value) const {
        return LongitudeDouble(value_ - value);
    }

    LongitudeDouble operator/(double value) const {
        return LongitudeDouble(value_ / value);
    }

    LongitudeDouble operator*(double value) const {
        return LongitudeDouble(value_ * value);
    }

    //======================================

    LongitudeDouble& operator+=(const eckit::Fraction& value) {
        value_ += double(value);
        return *this;
    }

    LongitudeDouble& operator-=(const eckit::Fraction& value){
        value_ -= double(value);
        return *this;
    }

    LongitudeDouble operator+(const eckit::Fraction& value) const {
        return LongitudeDouble(value_ + double(value));
    }

    LongitudeDouble operator-(const eckit::Fraction& value) const {
        return LongitudeDouble(value_ - double(value));
    }
    //========================================
    //========================================

    LongitudeDouble& operator+=(const LongitudeDouble& other) {
        value_ += other.value_;
        return *this;
    }

    LongitudeDouble& operator-=(const LongitudeDouble& other) {
        value_ -= other.value_;
        return *this;
    }

    LongitudeDouble operator+(const LongitudeDouble& other) const {
        return LongitudeDouble(value_ + other.value_);
    }

    LongitudeDouble operator-(const LongitudeDouble& other) const{
        return LongitudeDouble(value_ - other.value_);
    }

    bool operator==(const LongitudeDouble& other) const {
        return (*this) == other.value_;
    }

    bool operator!=(const LongitudeDouble& other) const{
        return (*this) != other.value_;
    }

    bool operator>(const LongitudeDouble& other) const {
     return (*this) > other.value_;
    }

    bool operator<(const LongitudeDouble& other) const  {
        return (*this) < other.value_;
    }

    bool operator>=(const LongitudeDouble& other) const{
        return (*this) >= other.value_;
    }

    bool operator<=(const LongitudeDouble& other) const{
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

    friend std::ostream&operator<<(std::ostream& s, const LongitudeDouble& p) {
        p.print(s);
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

    friend bool operator==(double, const LongitudeDouble& other);
    friend bool operator!=(double, const LongitudeDouble& other);

    friend bool operator>(double, const LongitudeDouble& other);
    friend bool operator<(double, const LongitudeDouble& other);

    friend bool operator>=(double, const LongitudeDouble& other);
    friend bool operator<=(double, const LongitudeDouble& other);

    friend LongitudeDouble operator+(double, const LongitudeDouble& other);

    friend LongitudeDouble operator-(double value, const LongitudeDouble& l) {
        return LongitudeDouble(value - l.value_);
    }

};



}  // namespace mir


#endif

