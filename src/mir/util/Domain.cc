/*
 * (C) Copyright 1996-2017 ECMWF.
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
    north_(north), west_(west), south_(south), east_(east) {
    normalise();
    check();
}


Domain Domain::makeGlobal() {
    return Domain(Latitude::NORTH_POLE, Longitude::GREENWICH,
                  Latitude::SOUTH_POLE, Longitude::GLOBE);
}


bool Domain::contains(const Latitude& lat, const Longitude& lon) const {
        return (lat <= north_) &&
               (lat >= south_) &&
                (lon.normalise(west_) <= east_);

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

void Domain::normalise() {
    Longitude eastNormalised = east_.normalise(west_);

    if (west_ != east_) {
        if (eastNormalised == west_) {
            eastNormalised += Longitude::GLOBE;
        }
        east_ = eastNormalised;
    }

    ASSERT(west_ <= east_);
    ASSERT(east_ <= west_ + Longitude::GLOBE);
}


void Domain::check() {
    ASSERT(north_ >= south_);
    ASSERT(north_ <= Latitude::NORTH_POLE);
    ASSERT(south_ >= Latitude::SOUTH_POLE);

    ASSERT(east_ - west_ >= 0);
    ASSERT(east_ - west_ <= Longitude::GLOBE);
}


} // namespace util
} // namespace mir
