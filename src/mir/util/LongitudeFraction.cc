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


#include "mir/util/LongitudeFraction.h"

#include <ostream>

#include "eckit/serialisation/Stream.h"
#include "eckit/utils/MD5.h"


namespace mir {


const LongitudeFraction LongitudeFraction::GLOBE(360);
const LongitudeFraction LongitudeFraction::DATE_LINE(180);
const LongitudeFraction LongitudeFraction::MINUS_DATE_LINE(-180);
const LongitudeFraction LongitudeFraction::GREENWICH(0);


void LongitudeFraction::print(std::ostream& out) const {
    out << double(value_);
}


bool LongitudeFraction::operator==(double value) const {
    return value_ == value;
}


bool LongitudeFraction::operator!=(double value) const {
    return value_ != value;
}


bool LongitudeFraction::operator>(double value) const {
    return value_ > value;
}


bool LongitudeFraction::operator<(double value) const {
    return value_ < value;
}


bool LongitudeFraction::operator>=(double value) const {
    return value_ >= value;
}


bool LongitudeFraction::operator<=(double value) const {
    return value_ <= value;
}


bool LongitudeFraction::operator==(const eckit::Fraction& value) const {
    return value_ == value;
}


bool LongitudeFraction::operator!=(const eckit::Fraction& value) const {
    return value_ != value;
}


bool LongitudeFraction::operator>(const eckit::Fraction& value) const {
    return value_ > value;
}


bool LongitudeFraction::operator<(const eckit::Fraction& value) const {
    return value_ < value;
}


bool LongitudeFraction::operator>=(const eckit::Fraction& value) const {
    return value_ >= value;
}


bool LongitudeFraction::operator<=(const eckit::Fraction& value) const {
    return value_ <= value;
}


bool LongitudeFraction::operator<(const LongitudeFraction& other) const {
    return value_ < other.value_;
}


bool LongitudeFraction::operator<=(const LongitudeFraction& other) const {
    return value_ <= other.value_;
}


bool LongitudeFraction::operator>(const LongitudeFraction& other) const {
    return value_ > other.value_;
}


bool LongitudeFraction::operator>=(const LongitudeFraction& other) const {
    return value_ >= other.value_;
}


bool LongitudeFraction::operator==(const LongitudeFraction& other) const {
    return value_ == other.value_;
}


bool LongitudeFraction::operator!=(const LongitudeFraction& other) const {
    return value_ != other.value_;
}


void LongitudeFraction::hash(eckit::MD5& md5) const {
    md5 << value_;
}


void LongitudeFraction::encode(eckit::Stream& s) const {
    s << value_;
}


void LongitudeFraction::decode(eckit::Stream& s) {
    s >> value_;
}


LongitudeFraction LongitudeFraction::normalise(const LongitudeFraction& minimum) const {
    LongitudeFraction lon(*this);
    while (lon < minimum) {
        lon += GLOBE;
    }
    while (lon >= minimum + GLOBE) {
        lon -= GLOBE;
    }
    return lon;
}


LongitudeFraction LongitudeFraction::distance(const LongitudeFraction& meridian) const {
    if (normalise(meridian) == meridian) {
        return 0;
    }

    LongitudeFraction d = (meridian < (*this) ? value_ - meridian.fraction() : meridian.fraction() - value_);
    while (d > LongitudeFraction::DATE_LINE) {
        d -= LongitudeFraction::DATE_LINE;
    }
    return d;
}


}  // namespace mir
