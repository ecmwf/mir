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


static void check(const Increments& inc) {
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

    long long n = (l - f) / i;

    return n + 1;
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
    return !(bbox.south().fraction() / south_north_).integer() ||
            !(bbox.west().fraction() / west_east_).integer();
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


bool Increments::multipleOf(const Increments& other) const {
    eckit::Fraction we = west_east_ / other.west_east_;
    eckit::Fraction ns = south_north_ / other.south_north_;
    return we.integer() && ns.integer();
}


void Increments::ratio(const Increments& other, size_t& we, size_t& ns) const {
    we = static_cast<long long>(west_east_ / other.west_east_);
    ns = static_cast<long long>(south_north_ / other.south_north_);
}


bool Increments::matches(const BoundingBox& bbox) const {
    eckit::Fraction we = (bbox.east() - bbox.west()).fraction() / west_east_;
    eckit::Fraction ns = (bbox.north() - bbox.south()).fraction() / south_north_;
    return we.integer() && ns.integer();
}


static eckit::Fraction multiple(const eckit::Fraction& box1,
                                const eckit::Fraction& box2,
                                const eckit::Fraction& inc) {

    static const eckit::Fraction min(1, 100);

    for (size_t i = 2; i < 1000; i++) {
        eckit::Fraction x = inc / i;
        if ((box1 / x).integer() && (box2 / x).integer()) {
            return x;
        }

        ASSERT(x > min);
    }
    NOTIMP;
}


Increments Increments::bestSubsetting(const BoundingBox& bbox) const {
    bool zero_zero = (bbox.north().fraction() /south_north_).integer()
                     && (bbox.south().fraction() /south_north_).integer()
                     && (bbox.west().fraction() /west_east_).integer()
                     && (bbox.east().fraction() /west_east_).integer();

    if (!zero_zero) {
        eckit::Fraction we = multiple(bbox.east().fraction(), bbox.west().fraction(), west_east_);
        eckit::Fraction ns = multiple(bbox.north().fraction(), bbox.south().fraction(), south_north_);
        return Increments(we, ns);
    }

    return *this;
}


static eckit::Fraction adjust(bool up, const eckit::Fraction& target, const eckit::Fraction& increment) {
    eckit::Fraction r = target / increment;

    eckit::Fraction::value_type n = r.integralPart();
    if (!r.integer() && (r > 0) == up) {
        n += (up ? 1 : -1);
    }

    return n * increment;
}


void Increments::globaliseBoundingBox(BoundingBox& bbox) const {
    using eckit::Fraction;

    // shift
    ASSERT(south_north_ > 0);
    ASSERT(west_east_ > 0);
    Fraction sn = (bbox.south().fraction() / south_north_).decimalPart() * south_north_;
    Fraction we = (bbox.west().fraction() / west_east_).decimalPart() * west_east_;


    // Latitude limits
    Fraction n = adjust(false, Latitude::NORTH_POLE.fraction() - sn, south_north_) + sn;
    Fraction s = adjust(true,  Latitude::SOUTH_POLE.fraction() - sn, south_north_) + sn;


    // Longitude limits
    // - West for non-periodic grids is not corrected!
    // - East for periodic grids is W + 360 - increment
    ASSERT(bbox.east() >= bbox.west());

    Fraction w = bbox.west().fraction();
    if (isPeriodic()) {
        w = adjust(true, Longitude::GREENWICH.fraction() - we, west_east_) + we;
    }

    Fraction e = adjust(false, w + Longitude::GLOBE.fraction(), west_east_);
    if (e - w == Longitude::GLOBE.fraction()) {
        e -= west_east_;
    }

    bbox = BoundingBox(n, w, s, e);
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


}  // namespace util
}  // namespace mir

