// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/util/LongitudeDouble.h"


#include <ostream>

#include "eckit/serialisation/Stream.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

namespace mir {

LongitudeDouble const LongitudeDouble::GLOBE(360);
LongitudeDouble const LongitudeDouble::DATE_LINE(180);
LongitudeDouble const LongitudeDouble::MINUS_DATE_LINE(-180);
LongitudeDouble const LongitudeDouble::GREENWICH(0);

void LongitudeDouble::print(std::ostream& out) const {
    out << value_;
}

bool LongitudeDouble::operator<(double value) const {
    return value_ < value;
}

bool LongitudeDouble::operator<=(double value) const {
    return eckit::types::is_approximately_lesser_or_equal(value_, value);
}

bool LongitudeDouble::operator>(double value) const {
    return value_ > value;
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

LongitudeDouble LongitudeDouble::distance(const LongitudeDouble& meridian) const {
    if (eckit::types::is_approximately_equal(normalise(meridian).value(), meridian.value())) {
        return 0.;
    }

    LongitudeDouble d = (meridian < (*this) ? value_ - meridian : meridian - value_);
    while (d > LongitudeDouble::DATE_LINE) {
        d -= LongitudeDouble::GLOBE;
    }
    return d;
}

}  // namespace mir
