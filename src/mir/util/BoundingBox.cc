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


namespace mir {
namespace util {


typedef eckit::FloatCompare<double> cmp;


BoundingBox::BoundingBox():
    north_(90), west_(0), south_(-90), east_(360) {
    normalise();
}


BoundingBox::BoundingBox(double north, double west, double south, double east) :
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


void BoundingBox::fill(grib_info &info) const  {
    // Warning: scanning mode not considered
    info.grid.latitudeOfFirstGridPointInDegrees  = north_;
    info.grid.longitudeOfFirstGridPointInDegrees = west_;
    info.grid.latitudeOfLastGridPointInDegrees   = south_;
    info.grid.longitudeOfLastGridPointInDegrees  = east_;
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
    while (east_ > 360) {
        east_ -= 360;
        west_ -= 360;
    }

    while (east_ < -180) {
        east_ += 360;
        west_ += 360;
    }

    while (east_ < west_) {
        east_ += 360;
    }

    ASSERT(north_ >= south_);
    ASSERT(west_ <= east_);
}


double BoundingBox::normalise(double lon) const {
    while (lon > east_) {
        lon -= 360;
    }

    while (lon < west_) {
        lon += 360;
    }
    return lon;
}


bool BoundingBox::contains(double lat, double lon) const {
    lon = normalise(lon);
    return cmp::isApproximatelyGreaterOrEqual(north_, lat) &&
           cmp::isApproximatelyGreaterOrEqual(lat, south_) &&
           cmp::isApproximatelyGreaterOrEqual(lon , west_) &&
           cmp::isApproximatelyGreaterOrEqual(east_, lon);
}


}  // namespace util
}  // namespace mir

