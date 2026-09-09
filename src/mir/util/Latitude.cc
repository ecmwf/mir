// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/util/Latitude.h"

#include <ostream>

#include "eckit/serialisation/Stream.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

namespace mir {

Latitude const Latitude::GLOBE(180);
Latitude const Latitude::NORTH_POLE(90);
Latitude const Latitude::SOUTH_POLE(-90);
Latitude const Latitude::EQUATOR(0);

void Latitude::print(std::ostream& out) const {
    out << value_;
}

bool Latitude::operator<(double value) const {
    return eckit::types::is_strictly_greater(value, value_);
}

bool Latitude::operator<=(double value) const {
    return eckit::types::is_approximately_lesser_or_equal<double>(value_, value);
}

bool Latitude::operator>(double value) const {
    return eckit::types::is_strictly_greater(value_, value);
}

bool Latitude::operator>=(double value) const {
    return eckit::types::is_approximately_greater_or_equal<double>(value_, value);
}

bool Latitude::operator==(double value) const {
    return eckit::types::is_approximately_equal<double>(value_, value);
}

bool Latitude::operator!=(double value) const {
    return !eckit::types::is_approximately_equal<double>(value_, value);
}

void Latitude::hash(eckit::MD5& md5) const {
    md5 << value_;
}

Latitude Latitude::distance(const Latitude& parallel) const {
    return parallel < (*this) ? value_ - parallel : parallel - value_;
}

void Latitude::encode(eckit::Stream& s) const {
    s << value_;
}

void Latitude::decode(eckit::Stream& s) {
    s >> value_;
}

}  // namespace mir
