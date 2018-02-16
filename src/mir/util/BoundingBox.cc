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


namespace {


static void check(const BoundingBox& bbox) {
    ASSERT(bbox.north() >= bbox.south());
    ASSERT(bbox.north() <= Latitude::NORTH_POLE);
    ASSERT(bbox.south() >= Latitude::SOUTH_POLE);

    ASSERT(bbox.east() - bbox.west() >= 0);
    ASSERT(bbox.east() - bbox.west() <= Longitude::GLOBE);
}


}  // (anonymous namespace)


BoundingBox::BoundingBox() :
    north_(Latitude::NORTH_POLE),
    west_(Longitude::GREENWICH),
    south_(Latitude::SOUTH_POLE),
    east_(Longitude::GLOBE) {
    check(*this);
}


BoundingBox::BoundingBox(const Latitude& north,
                         const Longitude& west,
                         const Latitude& south,
                         const Longitude& east) :
    north_(north), west_(west), south_(south), east_(east) {
    normalise();
    check(*this);
}


BoundingBox::BoundingBox(const param::MIRParametrisation& parametrisation) {
    ASSERT(parametrisation.get("north", north_));
    ASSERT(parametrisation.get("west",  west_ ));
    ASSERT(parametrisation.get("south", south_));
    ASSERT(parametrisation.get("east",  east_ ));

    normalise();
    check(*this);
}


BoundingBox::BoundingBox(const BoundingBox& other) {
    operator=(other);
}


BoundingBox::~BoundingBox() {
}


void BoundingBox::print(std::ostream& out) const {
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


bool BoundingBox::contains(const Latitude& lat, const Longitude& lon) const {
    return (lat <= north_) &&
           (lat >= south_) &&
            (lon.normalise(west_) <= east_);
}


void BoundingBox::makeName(std::ostream& out) const {
    out << "-" << north_
        << ":" << west_
        << ":" << south_
        << ":" << east_;
}


}  // namespace util
}  // namespace mir

