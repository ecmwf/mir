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


#include "mir/util/LongitudeDouble.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"
#include "eckit/serialisation/Stream.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/api/MIRJob.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Grib.h"

namespace mir {


LongitudeDouble LongitudeDouble::GLOBE(360); // 360
LongitudeDouble LongitudeDouble::DATE_LINE(180); // 180
LongitudeDouble LongitudeDouble::MINUS_DATE_LINE(-180); // -180
LongitudeDouble LongitudeDouble::GREENWICH(0); // 0

void LongitudeDouble::print(std::ostream& out) const {
    out << value_;
}

bool LongitudeDouble::operator<(double value) const {
    return eckit::types::is_strictly_greater(value, value_);
}

bool LongitudeDouble::operator<=(double value) const {
    return eckit::types::is_approximately_lesser_or_equal(value_, value);
}

bool LongitudeDouble::operator>(double value) const {
    return eckit::types::is_strictly_greater(value_, value);
}

bool LongitudeDouble::operator>=(double value) const {
    return eckit::types::is_approximately_greater_or_equal(value_, value);
}

bool LongitudeDouble::operator==(double value) const {
    return eckit::types::is_approximately_equal(value_, value);
}

bool LongitudeDouble::operator!=(double value) const {
    return !eckit::types::is_approximately_equal(value_, value);
}

void LongitudeDouble::hash(eckit::MD5& md5) const {
    md5 << value_;
}

void LongitudeDouble::encode(eckit::Stream& s) const {
    s << value_;
}

void LongitudeDouble::decode(eckit::Stream& s) {
    s >> value_;
}

bool LongitudeDouble::sameWithGrib1Accuracy(const LongitudeDouble& other) const {
    const double GRIB1EPSILON = 0.001;
    eckit::types::CompareApproximatelyEqual<double> cmp(GRIB1EPSILON);
    return cmp(value_, other.value_);
}

LongitudeDouble LongitudeDouble::normalise(const LongitudeDouble& minimum) const {
    LongitudeDouble lon(*this);
    while (lon < minimum) {
        lon += GLOBE;
    }
    while (lon >= minimum + GLOBE) {
        lon -= GLOBE;
    }
    return lon;
}


}  // namespace mir

