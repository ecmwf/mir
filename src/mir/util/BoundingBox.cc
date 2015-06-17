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

#include "mir/util/BoundingBox.h"

#include <iostream>

#include "mir/util/Compare.h"
#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Grib.h"

namespace mir {
namespace util {

BoundingBox::BoundingBox():
    north_(90),
    west_(0),
    south_(-90),
    east_(360),
    global_(true) {
    normalise();
}


BoundingBox::BoundingBox(double north,
                         double west,
                         double south,
                         double east,
                         bool global):
    north_(north),
    west_(west),
    south_(south),
    east_(east),
    global_(global) {
    normalise();
}

BoundingBox BoundingBox::Global(double north,
                         double west,
                         double south,
                         double east) {
    return BoundingBox(north, west, south, east, true);
}


BoundingBox::BoundingBox(double north,
                         double west,
                         double south,
                         double east):
    north_(north),
    west_(west),
    south_(south),
    east_(east),
    global_((north - south) == 180 && (west - east) == 360) {
    normalise();
}

BoundingBox::BoundingBox(const param::MIRParametrisation &parametrisation) {

    ASSERT(parametrisation.get("north", north_));
    ASSERT(parametrisation.get("west", west_));
    ASSERT(parametrisation.get("south", south_));
    ASSERT(parametrisation.get("east", east_));

    bool global = false;
    if (parametrisation.get("global", global)) {
        global_ = global;
    } else {
        // It seems that grib_api does not always provide this information
        eckit::Log::warning() << "Cannot establish if bounding box is global from "
                              << parametrisation
                              << ", assuming non-global"
                              << std::endl;
        global_ = false;
    }

    normalise();
}

BoundingBox::~BoundingBox() {
}

void BoundingBox::print(std::ostream &out) const {
    out << "BoundingBox["

        << "north=" << north_
        << ",west=" << west_
        << ",south=" << south_
        << ",east=" << east_
        << ", global=" << global_
        << "]";
}

void BoundingBox::fill(grib_info &info) const  {

    // Warning: scanning mode not considered

    info.grid.longitudeOfFirstGridPointInDegrees = west_;
    info.grid.longitudeOfLastGridPointInDegrees = east_;

    info.grid.latitudeOfFirstGridPointInDegrees = north_;
    info.grid.latitudeOfLastGridPointInDegrees = south_;
}

void BoundingBox::normalise() {
    while (east_ >= 360) {
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
    return eckit::FloatCompare<double>::isGreaterApproxEqual(north_, lat) &&
           eckit::FloatCompare<double>::isGreaterApproxEqual(lat, south_) &&
           eckit::FloatCompare<double>::isGreaterApproxEqual(lon , west_) &&
           eckit::FloatCompare<double>::isGreaterApproxEqual(east_, lon);
}


}  // namespace data
}  // namespace mir

