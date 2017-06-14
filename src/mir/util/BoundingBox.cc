/*
 * (C) Copyright 1996-2016 ECMWF.
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


#include "mir/util/BoundingBox.h"

#include <iostream>
#include "eckit/exception/Exceptions.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"
#include "mir/api/MIRJob.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Grib.h"
#include "mir/util/Increments.h"
#include "eckit/types/Fraction.h"


namespace mir {
namespace util {

const Longitude BoundingBox::THREE_SIXTY(360);
const Longitude BoundingBox::MINUS_ONE_EIGHTY(-180);
const Longitude BoundingBox::LON_ZERO(0);
const Latitude BoundingBox::SOUTH_POLE(-90);
const Latitude BoundingBox::NORTH_POLE(90);


BoundingBox::BoundingBox() :
    north_(NORTH_POLE),
    west_(LON_ZERO),
    south_(SOUTH_POLE),
    east_(THREE_SIXTY) {
    normalise();
}


BoundingBox::BoundingBox(const Latitude& north,
                         const Longitude& west,
                         const Latitude& south,
                         const Longitude& east) :
    north_(north), west_(west), south_(south), east_(east) {
    normalise();
}

BoundingBox::BoundingBox(const param::MIRParametrisation &parametrisation) {
    ASSERT(parametrisation.get("north", north_));
    ASSERT(parametrisation.get("west",  west_ ));
    ASSERT(parametrisation.get("south", south_));
    ASSERT(parametrisation.get("east",  east_ ));
    normalise();
}


BoundingBox::BoundingBox(const BoundingBox& other) {
    operator=(other);
}


BoundingBox::~BoundingBox() {
}


void BoundingBox::print(std::ostream &out) const {
    out << "BoundingBox["
        <<  "north=" << double(north_)
        << ",west=" << double(west_)
        << ",south=" << double(south_)
        << ",east=" << double(east_)
        << "]";
}



const double ROUNDING = 1e14;

static double rounded(double x) {
    return round(x * ROUNDING) / ROUNDING;
}


void BoundingBox::fill(grib_info &info) const  {
    // Warning: scanning mode not considered
    info.grid.latitudeOfFirstGridPointInDegrees  = rounded(north_);
    info.grid.longitudeOfFirstGridPointInDegrees = rounded(west_);
    info.grid.latitudeOfLastGridPointInDegrees   = rounded(south_);
    info.grid.longitudeOfLastGridPointInDegrees  = rounded(east_);
}


void BoundingBox::hash(eckit::MD5 &md5) const {
    md5.add(north_);
    md5.add(west_);
    md5.add(south_);
    md5.add(east_);
}


void BoundingBox::fill(api::MIRJob &job) const  {
    job.set("area", north_, west_, south_, east_);
}


void BoundingBox::normalise() {

    bool same = west_ == east_;

    ASSERT(north_ <= NORTH_POLE && south_ >= SOUTH_POLE);
    ASSERT(north_ >= south_);

    while (west_ < MINUS_ONE_EIGHTY) {
        west_ += THREE_SIXTY;
    }

    while (west_ >= THREE_SIXTY) {
        west_ -= THREE_SIXTY;
    }

    while (east_ < west_) {
        east_ += THREE_SIXTY;
    }

    while ((east_  - west_ ) > THREE_SIXTY) {
        east_ -= THREE_SIXTY;
    }

    if (same) {
        east_ = west_;
    }

    ASSERT(west_ <= east_);
}


Longitude BoundingBox::normalise(Longitude lon) const {

    while (eckit::types::is_strictly_greater(lon, east_)) {
        lon -= THREE_SIXTY;
    }

    while (eckit::types::is_strictly_greater(west_, lon)) {
        lon += THREE_SIXTY;
    }

    return lon;
}


bool BoundingBox::contains(const Latitude& lat, const Longitude& lon) const {
    const Longitude nlon = normalise(lon);
    return eckit::types::is_approximately_greater_or_equal(north_, lat) &&
           eckit::types::is_approximately_greater_or_equal(lat, south_) &&
           eckit::types::is_approximately_greater_or_equal(nlon, west_) &&
           eckit::types::is_approximately_greater_or_equal(east_, nlon);
}


template<class T, class U>
static size_t computeN(const T& first, const T& last, const U& inc) {
    ASSERT(first <= last);
    ASSERT(inc > 0);

    long long n = (last- first) / inc;

    return n + 1;
}

size_t BoundingBox::computeNi(const util::Increments& increments) const {
    return computeN(west_, east_, increments.west_east());
}

size_t BoundingBox::computeNj(const util::Increments& increments) const {
    return computeN(south_, north_, increments.south_north());
}

void BoundingBox::makeName(std::ostream& out) const {
    out << "-"
        << double(north_)
        << ":"
        << double(west_)
        << ":"
        << double(south_)
        << ":"
        << double(east_);
}



}  // namespace util
}  // namespace mir

