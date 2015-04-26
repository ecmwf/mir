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

#include "eckit/exception/Exceptions.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Grib.h"

namespace mir {
namespace util {


BoundingBox::BoundingBox(double north,
                         double west,
                         double south,
                         double east):
    north_(north),
    west_(west),
    south_(south),
    east_(east) {
    normalise();
}

BoundingBox::BoundingBox(const param::MIRParametrisation &parametrisation) {

    ASSERT(parametrisation.get("north", north_));
    ASSERT(parametrisation.get("west", west_));
    ASSERT(parametrisation.get("south", south_));
    ASSERT(parametrisation.get("east", east_));

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
}

bool BoundingBox::contains(double lat, double lon) const {
    while (lat >= 360) {
        lat -= 360;
    }
    while (lat < -180) {
        lat += 360;
    }
    return (lat <= north_) && (lat >= south_) && (lon >= west_) && (lon <= east_);
}

bool BoundingBox::global() const {
    // This is not supposed to be modified (e.g. don't add an epsilon in the comparaisons)
    return (north_ - south_) == 180 && (east_ - west_) == 360;
}

}  // namespace data
}  // namespace mir

