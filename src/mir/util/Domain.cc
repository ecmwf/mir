// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/util/Domain.h"

#include <ostream>

#include "eckit/types/FloatCompare.h"


namespace mir::util {


bool Domain::includesPoleNorth() const {
    return eckit::types::is_approximately_equal(north(), NORTH_POLE, PointLonLat::EPS);
}


bool Domain::includesPoleSouth() const {
    return eckit::types::is_approximately_equal(south(), SOUTH_POLE, PointLonLat::EPS);
}


Domain::operator atlas::RectangularDomain() const {
    return atlas::RectangularDomain({west(), east()}, {south(), north()});
}


void Domain::print(std::ostream& os) const {
    os << "Domain["
       << "north=" << north() << ",west=" << west() << ",south=" << south() << ",east=" << east()
       << ",isGlobal=" << isGlobal() << "]";
}


}  // namespace mir::util
