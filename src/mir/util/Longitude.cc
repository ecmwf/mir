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

#include "mir/util/Longitude.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "atlas/grid.h"
#include "atlas/util/Config.h"
#include "mir/api/MIRJob.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Grib.h"
#include "eckit/serialisation/Stream.h"
#include "eckit/utils/MD5.h"

namespace mir {


Longitude Longitude::GLOBE(360); // 360
Longitude Longitude::DATE_LINE(180); // 180
Longitude Longitude::MINUS_DATE_LINE(-180); // -180
Longitude Longitude::GREENWICH(0); // 0

void Longitude::print(std::ostream& out) const {
    out << value_;
}

bool Longitude::operator<(double value) const {
    return eckit::types::is_strictly_greater(value, value_);
}

bool Longitude::operator<=(double value) const {
    return eckit::types::is_approximately_lesser_or_equal<double>(value_, value);
}

bool Longitude::operator>(double value) const {
    return eckit::types::is_strictly_greater(value_, value);
}

bool Longitude::operator>=(double value) const {
    return eckit::types::is_approximately_greater_or_equal<double>(value_, value);
}

bool Longitude::operator==(double value) const {
    return eckit::types::is_approximately_equal<double>(value_, value);
}

bool Longitude::operator!=(double value) const {
    return !eckit::types::is_approximately_equal<double>(value_, value);
}

void Longitude::hash(eckit::MD5& md5) const {
    md5 << value_;
}

void Longitude::encode(eckit::Stream& s) const {
    s << value_;
}

void Longitude::decode(eckit::Stream& s) {
    s >> value_;
}

}  // namespace mir

