/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor
 * does it submit to any jurisdiction.
 */


#include "mir/util/Domain.h"

#include "mir/api/Atlas.h"


namespace mir {
namespace util {


Domain::Domain(Latitude north, Longitude west, Latitude south, Longitude east) :
    BoundingBox(north, west, south, east) {
}


bool Domain::contains(const repres::Iterator::point_ll_t& p) const {
    return BoundingBox::contains(p.lat, p.lon);
}


bool Domain::contains(const Latitude& lat, const Longitude& lon) const {
    return BoundingBox::contains(lat, lon);
}


Domain Domain::makeGlobal() {
    return Domain(Latitude::NORTH_POLE, Longitude::GREENWICH,
                  Latitude::SOUTH_POLE, Longitude::GLOBE);
}


bool Domain::includesPoleNorth() const {
    return north() == Latitude::NORTH_POLE;
}


bool Domain::includesPoleSouth() const {
    return south() == Latitude::SOUTH_POLE;
}


bool Domain::isPeriodicEastWest() const {
    return east() - west() == Longitude::GLOBE;
}


Domain::operator atlas::RectangularDomain() const {
    return atlas::RectangularDomain(
        {{west().value(),  east().value()} },
        {{south().value(), north().value()} } );
}


void Domain::print(std::ostream& os) const {
    os << "Domain["
       <<  "north=" << north()
        << ",west="  << west()
        << ",south=" << south()
        << ",east="  << east()
        << ",isGlobal=" << isGlobal()
       // << ",includesPoleNorth=" << includesPoleNorth()
       // << ",includesPoleSouth=" << includesPoleSouth()
       // << ",isPeriodicEastWest=" << isPeriodicEastWest()
       << "]";
}


} // namespace util
} // namespace mir
