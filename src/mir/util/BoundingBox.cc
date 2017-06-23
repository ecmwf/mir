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
#include "eckit/utils/MD5.h"
#include "mir/api/MIRJob.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Grib.h"


namespace mir {
namespace util {


BoundingBox::BoundingBox() :
    north_(Latitude::NORTH_POLE),
    west_(Longitude::GREENWICH),
    south_(Latitude::SOUTH_POLE),
    east_(Longitude::GLOBE) {
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
        <<  "north=" << north_
        << ",west=" << west_
        << ",south=" << south_
        << ",east=" << east_
        << "]";
}



const double ROUNDING = 1e14;

static double rounded(double x) {
    return round(x * ROUNDING) / ROUNDING;
}


void BoundingBox::fill(grib_info &info) const  {
    // Warning: scanning mode not considered
    info.grid.latitudeOfFirstGridPointInDegrees  = rounded(north_.value());
    info.grid.longitudeOfFirstGridPointInDegrees = rounded(west_.value());
    info.grid.latitudeOfLastGridPointInDegrees   = rounded(south_.value());
    info.grid.longitudeOfLastGridPointInDegrees  = rounded(east_.value());
}


void BoundingBox::hash(eckit::MD5 &md5) const {
    md5.add(north_);
    md5.add(west_);
    md5.add(south_);
    md5.add(east_);
}


void BoundingBox::fill(api::MIRJob &job) const  {
    job.set("area", north_.value(), west_.value(), south_.value(), east_.value());
}


void BoundingBox::normalise() {

    bool same = west_ == east_;

    ASSERT(north_ <= Latitude::NORTH_POLE && south_ >= Latitude::SOUTH_POLE);
    ASSERT(north_ >= south_);

    while (west_ < Longitude::MINUS_DATE_LINE) {
        west_ += Longitude::GLOBE;
    }

    while (west_ >= Longitude::GLOBE) {
        west_ -= Longitude::GLOBE;
    }

    while (east_ <= west_) {
        east_ += Longitude::GLOBE;
    }

    while ((east_  - west_ ) > Longitude::GLOBE) {
        east_ -= Longitude::GLOBE;
    }

    if (same) {
        east_ = west_;
    }

    ASSERT(west_ <= east_);
}


Longitude BoundingBox::normalise(Longitude lon) const {

    while (lon > east_) {
        lon -= Longitude::GLOBE;
    }

    while (lon < west_) {
        lon += Longitude::GLOBE;
    }

    return lon;
}


bool BoundingBox::contains(const Latitude& lat, const Longitude& lon) const {
    const Longitude nlon = normalise(lon);
    return (lat <= north_) &&
           (lat >= south_) &&
           (nlon >= west_) &&
           (nlon <= east_);
}


void BoundingBox::makeName(std::ostream& out) const {
    out << "-"
        << north_
        << ":"
        << west_
        << ":"
        << south_
        << ":"
        << east_;
}


}  // namespace util
}  // namespace mir

