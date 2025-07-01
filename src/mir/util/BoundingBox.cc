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


#include "mir/util/BoundingBox.h"

#include <algorithm>
#include <ostream>

#include "eckit/log/JSON.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/api/MIRJob.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"


namespace mir::util {


static double get(const param::MIRParametrisation& param, const char* key) {
    double value = 0.;
    ASSERT(param.get(key, value));
    return value;
}


BoundingBox::BoundingBox() :
    north_(eckit::geo::NORTH_POLE.lat), west_(0), south_(eckit::geo::SOUTH_POLE.lat), east_(PointLonLat::FULL_ANGLE) {}


BoundingBox::BoundingBox(double north, double west, double south, double east) :
    north_(north), west_(west), south_(south), east_(east) {
    if (west_ != east_) {
        auto eastNormalised = PointLonLat::normalise_angle_to_minimum(east_, west_);
        if (eckit::types::is_approximately_equal(eastNormalised, west_)) {
            eastNormalised += PointLonLat::FULL_ANGLE;
        }
        east_ = eastNormalised;
    }

    ASSERT(west_ <= east_ && east_ <= west_ + PointLonLat::FULL_ANGLE);
    ASSERT(eckit::geo::SOUTH_POLE.lat <= south_ && south_ <= north_ && north_ <= eckit::geo::NORTH_POLE.lat);
}


BoundingBox::BoundingBox(const param::MIRParametrisation& param) :
    BoundingBox(get(param, "north"), get(param, "west"), get(param, "south"), get(param, "east")) {}


bool BoundingBox::operator==(const BoundingBox& other) const {
    return (north_ == other.north_) && (south_ == other.south_) && (west_ == other.west_) && (east_ == other.east_);
}


void BoundingBox::print(std::ostream& out) const {
    out << "BoundingBox["
        << "north=" << north_ << ",west=" << west_ << ",south=" << south_ << ",east=" << east_ << "]";
}


void BoundingBox::fillGrib(grib_info& info) const {
    // Warning: scanning mode not considered
    info.grid.latitudeOfFirstGridPointInDegrees  = north_;
    info.grid.longitudeOfFirstGridPointInDegrees = west_;
    info.grid.latitudeOfLastGridPointInDegrees   = south_;
    info.grid.longitudeOfLastGridPointInDegrees  = east_;

    info.extra_set("expandBoundingBox", 1L);
}


void BoundingBox::hash(eckit::MD5& md5) const {
    md5.add(north_);
    md5.add(west_);
    md5.add(south_);
    md5.add(east_);
}


void BoundingBox::fillJob(api::MIRJob& job) const {
    job.set("area", north_, west_, south_, east_);
}


bool BoundingBox::isPeriodicWestEast() const {
    return (west_ != east_) && (west_ == PointLonLat::normalise_angle_to_minimum(east_, west_));
}


bool BoundingBox::contains(const PointLonLat& p) const {
    // notice the order
    return contains(p.lat, p.lon);
}


bool BoundingBox::contains(double lat, double lon) const {
    return (lat <= north_) && (lat >= south_) && (PointLonLat::normalise_angle_to_minimum(lon, west_) <= east_);
}


bool BoundingBox::contains(const BoundingBox& other) const {

    if (other.empty()) {
        return contains(other.south(), other.west());
    }

    // check for West/East range (if non-periodic), then other's corners
    if (east_ - west_ < other.east() - other.west() ||
        east_ < PointLonLat::normalise_angle_to_minimum(other.east_, west_)) {
        return false;
    }

    return contains(other.north(), other.west()) && contains(other.north(), other.east()) &&
           contains(other.south(), other.west()) && contains(other.south(), other.east());
}


bool BoundingBox::intersects(BoundingBox& other) const {
    auto n = std::min(north_, other.north_);
    auto s = std::max(south_, other.south_);

    bool intersectsSN = s <= n;
    if (!intersectsSN) {
        n = s;
    }

    if (isPeriodicWestEast() && other.isPeriodicWestEast()) {
        other = {n, other.west_, s, other.east_};
        return intersectsSN;
    }

    auto w = std::min(west_, other.west_);
    auto e = w;

    auto intersect = [](const BoundingBox& a, const BoundingBox& b, double& w, double& e) {
        bool p = a.isPeriodicWestEast();
        if (p || b.isPeriodicWestEast()) {
            w = (p ? b : a).west_;
            e = (p ? b : a).east_;
            return true;
        }

        auto ref = PointLonLat::normalise_angle_to_minimum(b.west_, a.west_);
        auto w_  = std::max(a.west_, ref);
        auto e_  = std::min(a.east_, PointLonLat::normalise_angle_to_minimum(b.east_, ref));

        if (w_ <= e_) {
            w = w_;
            e = e_;
            return true;
        }
        return false;
    };

    bool intersectsWE = west_ <= other.west_ ? intersect(*this, other, w, e) || intersect(other, *this, w, e)
                                             : intersect(other, *this, w, e) || intersect(*this, other, w, e);

    ASSERT(w <= e);
    other = {n, w, s, e};

    return intersectsSN && intersectsWE;
}


bool BoundingBox::empty() const {
    return !eckit::types::is_strictly_greater(north_, south_) || !eckit::types::is_strictly_greater(east_, west_);
}


void BoundingBox::makeName(std::ostream& out) const {
    out << "-" << north_ << ":" << west_ << ":" << south_ << ":" << east_;
}

void BoundingBox::json(eckit::JSON& j) const {
    j.startObject();
    j << "north" << north_;
    j << "west" << west_;
    j << "south" << south_;
    j << "east" << east_;
    j.endObject();
}

}  // namespace mir::util
