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


#include "mir/util/Increments.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "mir/api/MIRJob.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Grib.h"


namespace mir {
namespace util {


namespace {


static void check(const Increments& /*inc*/) {
    // ASSERT(inc.west_east_ > 0);
    // ASSERT(inc.south_north_ > 0);
}


template<class T>
static size_t computeN(const T& first, const T& last, const eckit::Fraction& inc) {
    ASSERT(first <= last);
    ASSERT(inc > 0);

    eckit::Fraction l = last.fraction();
    eckit::Fraction f = first.fraction();
    eckit::Fraction i = inc;

    // std::cout << double(last) << " " << double(first) << " " << double(inc) << std::endl;

    // std::cout << l << " " << f << " " << i << std::endl;

    eckit::Fraction::value_type n = (l - f) / i;

    return size_t(n + 1);
}


}  // (anonymous namespace)


Increments::Increments(const param::MIRParametrisation& parametrisation) {
    ASSERT(parametrisation.get("west_east_increment", west_east_));
    ASSERT(parametrisation.get("south_north_increment", south_north_));
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


Increments::Increments(const eckit::Fraction& west_east, const eckit::Fraction& south_north) :
    west_east_(west_east),
    south_north_(south_north) {
    check(*this);
}


Increments::Increments(double west_east, const eckit::Fraction& south_north) :
    west_east_(west_east),
    south_north_(south_north) {
    check(*this);
}


Increments::Increments(const eckit::Fraction& west_east, double south_north) :
    west_east_(west_east),
    south_north_(south_north) {
    check(*this);
}


Increments::~Increments() {
}


bool Increments::isPeriodic() const {
    return (Longitude::GLOBE.fraction() / west_east_).integer();
}


bool Increments::isShifted(const BoundingBox& bbox) const {
    return isLatitudeShifted(bbox) || isLongitudeShifted(bbox);
}


void Increments::print(std::ostream& out) const {
    out << "Increments["
        << "west_east=" << double(west_east_)
        << ",south_north=" << double(south_north_)
        << "]";
}


void Increments::fill(grib_info& info) const  {
    // Warning: scanning mode not considered
    info.grid.iDirectionIncrementInDegrees = west_east_;
    info.grid.jDirectionIncrementInDegrees = south_north_;
}


void Increments::fill(api::MIRJob& job) const  {
    job.set("grid", west_east_, south_north_);
}


static eckit::Fraction adjust(bool up, const eckit::Fraction& target, const eckit::Fraction& increment) {
    eckit::Fraction r = target / increment;

    eckit::Fraction::value_type n = r.integralPart();
    if (!r.integer() && (r > 0) == up) {
        n += (up ? 1 : -1);
    }

    return n * increment;
}


void Increments::globaliseBoundingBox(BoundingBox& bbox, bool allowLongitudeShift, bool allowLatitudeShift) const {
    using eckit::Fraction;

    // Latitude limits

    ASSERT(south_north_ > 0);
    Fraction shift_sn = Fraction(0);
    if (allowLatitudeShift) {
        shift_sn = (bbox.south().fraction() / south_north_).decimalPart() * south_north_;
    }

    Fraction n = adjust(false, Latitude::NORTH_POLE.fraction() - shift_sn, south_north_) + shift_sn;
    Fraction s = adjust(true,  Latitude::SOUTH_POLE.fraction() - shift_sn, south_north_) + shift_sn;


    // Longitude limits
    // - West for non-periodic grids is not corrected!
    // - East for periodic grids is W + 360 - increment

    ASSERT(west_east_ > 0);
    Fraction shift_we = Fraction(0);
    if (allowLongitudeShift) {
        shift_we = (bbox.west().fraction() / west_east_).decimalPart() * west_east_;
    }

    Fraction w = bbox.west().fraction();
    if (isPeriodic()) {
        w = adjust(true, Longitude::GREENWICH.fraction() - shift_we, west_east_) + shift_we;
    }

    Fraction e = adjust(false, w + Longitude::GLOBE.fraction() - shift_we, west_east_) + shift_we;
    if (e - w == Longitude::GLOBE.fraction()) {
        e -= west_east_;
    }

    bbox = BoundingBox(n, w, s, e);

    ASSERT(allowLatitudeShift || !isLatitudeShifted(bbox));
    ASSERT(allowLongitudeShift || !isLongitudeShifted(bbox));
}


size_t Increments::computeNi(const BoundingBox& bbox) const {
    return computeN(bbox.west(), bbox.east(), west_east_);
}


size_t Increments::computeNj(const BoundingBox& bbox) const {
    return computeN(bbox.south(), bbox.north(), south_north_);
}


void Increments::makeName(std::ostream& out) const {
    out << "-"
        << double(west_east_)
        << "x"
        << double(south_north_)
           ;
}


bool Increments::isLatitudeShifted(const BoundingBox& bbox) const {
    return !(bbox.south().fraction() / south_north_).integer();
}


bool Increments::isLongitudeShifted(const BoundingBox& bbox) const {
    return !(bbox.west().fraction() / west_east_).integer();
}


}  // namespace util
}  // namespace mir

