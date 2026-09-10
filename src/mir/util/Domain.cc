// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/util/Domain.h"

#include <ostream>


namespace mir::util {


bool Domain::includesPoleNorth() const {
    return north() == Latitude::NORTH_POLE;
}


bool Domain::includesPoleSouth() const {
    return south() == Latitude::SOUTH_POLE;
}


Domain::operator atlas::RectangularDomain() const {
    return atlas::RectangularDomain({west().value(), east().value()}, {south().value(), north().value()});
}


void Domain::print(std::ostream& os) const {
    os << "Domain["
       << "north=" << north() << ",west=" << west() << ",south=" << south() << ",east=" << east()
       << ",isGlobal=" << isGlobal() << "]";
}


}  // namespace mir::util
