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


#include "mir/util/Latitude.h"

#include <ostream>

#include "eckit/serialisation/Stream.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

namespace mir {

Latitude Latitude::GLOBE(180);
Latitude Latitude::NORTH_POLE(90);
Latitude Latitude::SOUTH_POLE(-90);
Latitude Latitude::EQUATOR(0);

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
