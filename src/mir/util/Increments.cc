/*
 * (C) Copyright 1996- ECMWF.
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


#include "mir/util/Increments.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "eckit/types/Fraction.h"
#include "mir/api/MIRJob.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Grib.h"


namespace mir {
namespace util {


namespace {


static void check(const Increments& inc) {
    ASSERT(inc.west_east().longitude() >= 0);
    ASSERT(inc.south_north().latitude() >= 0);
}


template<class T>
static size_t computeN(const T& first, const T& last, const T& inc) {
    ASSERT(first <= last);
    ASSERT(inc > 0);

    eckit::Fraction l = last.fraction();
    eckit::Fraction f = first.fraction();
    eckit::Fraction i = inc.fraction();
    eckit::Fraction r = (l - f) / i;

//    std::cout << "\t" << last
//              << "\t" << first
//              << "\t" << inc
//              << "\t" << l
//              << "\t" << f
//              << "\t" << i
//              << "\t" << double(l)
//              << "\t" << double(f)
//              << "\t" << double(i)
//              << std::endl;

//    eckit::Fraction::value_type n = r.integralPart();
    long long n = ((l - f) / i).integralPart();

    return size_t(n + 1);
}


template<class T>
static T adjust(bool up, const T& target, const T& inc) {
    ASSERT(inc > 0);

    eckit::Fraction i = inc.fraction();
    eckit::Fraction r = target.fraction() / i;

    eckit::Fraction::value_type n = r.integralPart();
    if (!r.integer() && (r > 0) == up) {
        n += (up ? 1 : -1);
    }

    return T(n * i);
}


}  // (anonymous namespace)


Increments::Increments(const param::MIRParametrisation& parametrisation) {
    Latitude lat;
    ASSERT(parametrisation.get("south_north_increment", lat));
    south_north_ = lat;

    Longitude lon;
    ASSERT(parametrisation.get("west_east_increment", lon));
    west_east_ = lon;

    check(*this);
}


Increments::Increments(const Increments& other) :
    west_east_(other.west_east_),
    south_north_(other.south_north_) {
    check(*this);
}


Increments::Increments(double west_east, double south_north) :
    west_east_(west_east),
    south_north_(south_north) {
    check(*this);
}


Increments::Increments(const LongitudeIncrement& west_east, const LatitudeIncrement& south_north) :
    west_east_(west_east),
    south_north_(south_north) {
    check(*this);
}


Increments::~Increments() = default;


bool Increments::operator==(const Increments& other) const {
    return  (west_east_.longitude() == other.west_east_.longitude()) &&
            (south_north_.latitude() == other.south_north_.latitude());
}


bool Increments::operator!=(const Increments& other) const {
    return  (west_east_.longitude() != other.west_east_.longitude()) ||
            (south_north_.latitude() != other.south_north_.latitude());
}


bool Increments::isPeriodic() const {
    return (Longitude::GLOBE.fraction() / west_east_.longitude().fraction()).integer();
}


bool Increments::isShifted(const BoundingBox& bbox) const {
    return isLatitudeShifted(bbox) || isLongitudeShifted(bbox);
}


void Increments::print(std::ostream& out) const {
    out << "Increments["
        << "west_east=" << west_east_.longitude()
        << ",south_north=" << south_north_.latitude()
        << "]";
}


void Increments::fill(grib_info& info) const  {
    // Warning: scanning mode not considered
    info.grid.iDirectionIncrementInDegrees = west_east_.longitude().value();
    info.grid.jDirectionIncrementInDegrees = south_north_.latitude().value();
}


void Increments::fill(api::MIRJob& job) const  {
    job.set("grid", west_east_.longitude().value(), south_north_.latitude().value());
}


void Increments::globaliseBoundingBox(BoundingBox& bbox, bool allowLongitudeShift, bool allowLatitudeShift) const {
    const Latitude& sn = south_north_.latitude();
    const Longitude& we = west_east_.longitude();

    // Latitude limits

    ASSERT(sn > 0);
    LatitudeIncrement shift_sn(0);
    if (allowLatitudeShift) {
        shift_sn = (bbox.south().fraction() / sn.fraction()).decimalPart() * sn.fraction();
    }

    Latitude n = adjust(false, Latitude::NORTH_POLE - shift_sn.latitude(), sn) + shift_sn.latitude();
    Latitude s = adjust(true,  Latitude::SOUTH_POLE - shift_sn.latitude(), sn) + shift_sn.latitude();


    // Longitude limits
    // - West for non-periodic grids is not corrected!
    // - East for periodic grids is W + 360 - increment

    ASSERT(we > 0);
    LongitudeIncrement shift_we(0);
    if (allowLongitudeShift) {
        shift_we = (bbox.west().fraction() / we.fraction()).decimalPart() *  we.fraction();
    }

    Longitude w = bbox.west();
    if (isPeriodic()) {
        w = adjust(true, Longitude::GREENWICH - shift_we.longitude(), we) + shift_we.longitude();
    }

    Longitude e = adjust(false, w + Longitude::GLOBE - shift_we.longitude(), we) + shift_we.longitude();
    if (e - w == Longitude::GLOBE) {
        e -= we;
    }

    bbox = BoundingBox(n, w, s, e);

    ASSERT(allowLatitudeShift || !isLatitudeShifted(bbox));
    ASSERT(allowLongitudeShift || !isLongitudeShifted(bbox));
}


size_t Increments::computeNi(const BoundingBox& bbox) const {
    return computeN(bbox.west(), bbox.east(), west_east_.longitude());
}


size_t Increments::computeNj(const BoundingBox& bbox) const {
    return computeN(bbox.south(), bbox.north(), south_north_.latitude());
}


void Increments::makeName(std::ostream& out) const {
    out << "-" << west_east_.longitude().value()
        << "x" << south_north_.latitude().value();
}


bool Increments::isLatitudeShifted(const BoundingBox& bbox) const {
    return !(bbox.south().fraction() / south_north_.latitude().fraction()).integer();
}


bool Increments::isLongitudeShifted(const BoundingBox& bbox) const {
    return !(bbox.west().fraction() / west_east_.longitude().fraction()).integer();
}


}  // namespace util
}  // namespace mir

