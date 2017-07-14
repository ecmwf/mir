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

#include "eckit/types/FloatCompare.h"

#include "mir/util/LongitudeFraction.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"

#include "atlas/util/Config.h"
#include "mir/api/MIRJob.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Grib.h"
#include "eckit/serialisation/Stream.h"
#include "eckit/utils/MD5.h"

namespace mir {


LongitudeFraction LongitudeFraction::GLOBE(360); // 360
LongitudeFraction LongitudeFraction::DATE_LINE(180); // 180
LongitudeFraction LongitudeFraction::MINUS_DATE_LINE(-180); // -180
LongitudeFraction LongitudeFraction::GREENWICH(0); // 0

void LongitudeFraction::print(std::ostream& out) const {
    out << double(value_);
}

bool LongitudeFraction::operator<(double value) const {
    return value_ < value;
}

bool LongitudeFraction::operator<=(double value) const {
    return value_ <= value;
}

bool LongitudeFraction::operator>(double value) const {
    return value_ > value;
}

bool LongitudeFraction::operator>=(double value) const {
   return value_ >= value;
}

bool LongitudeFraction::operator==(double value) const {
    return value_ == value;
}

bool LongitudeFraction::operator!=(double value) const {
    return value_ != value;
}

//=========

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

bool LongitudeFraction::sameWithGrib1Accuracy(const LongitudeFraction& other) const {
    const double GRIB1EPSILON = 0.001;
    eckit::types::CompareApproximatelyEqual<double> cmp(GRIB1EPSILON);
    return cmp(value_, other.value_);
}


}  // namespace mir

