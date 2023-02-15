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


#include "mir/util/Increments.h"

#include <ostream>

#include "eckit/types/Fraction.h"

#include "mir/api/MIRJob.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"


namespace mir::util {


static void check(const Increments& inc) {
    ASSERT(inc.west_east().longitude() >= 0);
    ASSERT(inc.south_north().latitude() >= 0);
}


Increments::Increments() = default;


Increments::Increments(const param::MIRParametrisation& parametrisation) {
    double south_north;
    ASSERT(parametrisation.get("south_north_increment", south_north));
    south_north_ = south_north;

    double west_east;
    ASSERT(parametrisation.get("west_east_increment", west_east));
    west_east_ = west_east;

    check(*this);
}


Increments::Increments(const Increments& other) : Increments(other.west_east_, other.south_north_) {}


Increments::Increments(double westEastIncrement, double southNorthIncrement) :
    Increments(LongitudeIncrement(westEastIncrement), LatitudeIncrement(southNorthIncrement)) {}


Increments::Increments(const LongitudeIncrement& west_east, const LatitudeIncrement& south_north) :
    west_east_(west_east), south_north_(south_north) {
    check(*this);
}


bool Increments::operator==(const Increments& other) const {
    return (west_east_.longitude() == other.west_east_.longitude()) &&
           (south_north_.latitude() == other.south_north_.latitude());
}


bool Increments::operator!=(const Increments& other) const {
    return (west_east_.longitude() != other.west_east_.longitude()) ||
           (south_north_.latitude() != other.south_north_.latitude());
}


Increments& Increments::operator=(const Increments&) = default;


bool Increments::isPeriodic() const {
    return (Longitude::GLOBE.fraction() / west_east_.longitude().fraction()).integer();
}


bool Increments::isShifted(const BoundingBox& bbox) const {
    const PointLatLon p{bbox.south(), bbox.west()};
    return isLatitudeShifted(p) || isLongitudeShifted(p);
}


bool Increments::isLatitudeShifted(const BoundingBox& bbox) const {
    return isLatitudeShifted(PointLatLon{bbox.south(), bbox.west()});
}


bool Increments::isLongitudeShifted(const BoundingBox& bbox) const {
    return isLongitudeShifted(PointLatLon{bbox.south(), bbox.west()});
}


bool Increments::isShifted(const PointLatLon& p) const {
    return isLatitudeShifted(p) || isLongitudeShifted(p);
}


bool Increments::isLatitudeShifted(const PointLatLon& p) const {
    const auto& inc = south_north_.latitude();
    if (inc == 0) {
        return false;
    }
    return !(p.lat().fraction() / inc.fraction()).integer();
}


bool Increments::isLongitudeShifted(const PointLatLon& p) const {
    const auto& inc = west_east_.longitude();
    if (inc == 0) {
        return false;
    }
    return !(p.lon().fraction() / inc.fraction()).integer();
}


void Increments::print(std::ostream& out) const {
    out << "Increments["
        << "west_east=" << west_east_.longitude() << ",south_north=" << south_north_.latitude() << "]";
}


void Increments::fillGrib(grib_info& info) const {
    // Warning: scanning mode not considered
    info.grid.iDirectionIncrementInDegrees = west_east_.longitude().value();
    info.grid.jDirectionIncrementInDegrees = south_north_.latitude().value();
}


void Increments::fillJob(api::MIRJob& job) const {
    job.set("grid", west_east_.longitude().value(), south_north_.latitude().value());
}


void Increments::makeName(std::ostream& out) const {
    out << "-" << west_east_.longitude().value() << "x" << south_north_.latitude().value();
}


}  // namespace mir::util
