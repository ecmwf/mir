/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/util/Projection.h"

// #include <ostream>
// #include <vector>

// #include "eckit/types/FloatCompare.h"
#include "eckit/geo/area/BoundingBox.h"

// #include "mir/api/MIRJob.h"
// #include "mir/param/MIRParametrisation.h"
// #include "mir/util/Atlas.h"
#include "mir/util/BoundingBox.h"
// #include "mir/util/Exceptions.h"
// #include "mir/util/Grib.h"


namespace eckit::geo {
namespace area {
class BoundingBox;
}
namespace util {
area::BoundingBox bounding_box(Point2 min, Point2 max, const Projection&);
}
}  // namespace eckit::geo


namespace mir::util {


Projection::Spec Projection::spec() const {
    return spec_;
}


void Projection::hash(eckit::Hash& h) const {
    spec_.hash(h);
}


Projection::Projection(const Spec& spec) : spec_(spec) {
    ASSERT(proj_);
}


Projection::operator bool() const {
    return true;
}


Point2 Projection::xy(const Point2& p) const {
    return p;
}


PointLonLat Projection::lonlat(const Point2& p) const {
    return {p.X, p.Y};
}


BoundingBox Projection::lonlatBoundingBox(Point2 min, Point2 max) const {
    // use [0, 360[ longitude range if periodic
    auto after = eckit::geo::util::bounding_box(min, max, *proj_);
    return {after.north, after.periodic() ? Longitude::GREENWICH : after.west, after.south,
            after.periodic() ? Longitude::GLOBE : after.east};
}


}  // namespace mir::util
