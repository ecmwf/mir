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


#include "mir/util/Domain.h"

#include <ostream>


namespace mir::util {


bool Domain::includesPoleNorth() const {
    return PointLonLat{0, north()}.north_pole();
}


bool Domain::includesPoleSouth() const {
    return PointLonLat{0, south()}.south_pole();
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
