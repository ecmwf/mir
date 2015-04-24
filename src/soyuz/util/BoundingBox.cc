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


#include <iostream>

#include "eckit/exception/Exceptions.h"

#include "soyuz/util/BoundingBox.h"
#include "soyuz/repres/Representation.h"
#include "soyuz/util/Grib.h"
#include "soyuz/param/MIRParametrisation.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/utils/Translator.h"

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
}

BoundingBox::BoundingBox(const param::MIRParametrisation &parametrisation) {

    eckit::Translator<std::string, double> s2d;
    std::string value;

    ASSERT(parametrisation.get("north", value));
    north_ = s2d(value);

    ASSERT(parametrisation.get("west", value));
    west_ = s2d(value);

    ASSERT(parametrisation.get("south", value));
    south_ = s2d(value);

    ASSERT(parametrisation.get("east", value));
    east_ = s2d(value);
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

BoundingBox BoundingBox::intersection(const BoundingBox &other) const {
    double n = std::min(north_, other.north_);
    double s = std::max(south_, other.south_);
    double w = std::max(west_, other.west_);
    double e = std::min(east_, other.east_);
    return BoundingBox(n, w, s, e);
}


}  // namespace data
}  // namespace mir

