/*
 * (C) Copyright 1996-2017 ECMWF.
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


#include "mir/util/RotatedIterator.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/geometry/Point2.h"
#include "atlas/util/Config.h"


namespace mir {
namespace util {


RotatedIterator::RotatedIterator(Iterator* iterator, const util::Rotation& rotation) :
    iterator_(iterator),
    rotation_(rotation) {

    // Setup projection using South Pole rotated position, as seen from the non-rotated frame
    const eckit::geometry::LLPoint2 pole(rotation_.south_pole_longitude(), rotation_.south_pole_latitude());

    atlas::util::Config config;
    config.set("type", "rotated_lonlat");
    config.set("south_pole", std::vector<double>({pole.lon(), pole.lat()}));
    config.set("rotation_angle", rotation_.south_pole_rotation_angle());

    projection_ = atlas::Projection(config);
}


RotatedIterator::~RotatedIterator() {
}


void RotatedIterator::print(std::ostream& out) const {
    out << "RotatedIterator[iterator=" << *iterator_ << ",rotation=" << rotation_ << "]";
}


bool RotatedIterator::next(Latitude &lat, Longitude &lon) {
    if (iterator_->next(lat, lon)) {

        // use Point2 to avoid the LLPoint2 normalising, notice the order
        eckit::geometry::Point2 p = projection_.lonlat(eckit::geometry::Point2(lon, lat));

        lat = p[eckit::geometry::LAT];
        lon = p[eckit::geometry::LON];
        return true;
    }
    return false;
}


}  // namespace util
}  // namespace mir

