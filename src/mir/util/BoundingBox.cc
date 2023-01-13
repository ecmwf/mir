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

#include "eckit/types/FloatCompare.h"
#include "eckit/types/Fraction.h"
#include "eckit/utils/MD5.h"

#include "mir/api/MIRJob.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"


namespace mir {
namespace util {


static void check(const BoundingBox& bbox) {
    ASSERT(bbox.north() >= bbox.south());
    ASSERT(bbox.north() <= Latitude::NORTH_POLE);
    ASSERT(bbox.south() >= Latitude::SOUTH_POLE);

    ASSERT(bbox.east() - bbox.west() >= 0);
    ASSERT(bbox.east() - bbox.west() <= Longitude::GLOBE);
}


BoundingBox::BoundingBox() :
    north_(Latitude::NORTH_POLE), west_(Longitude::GREENWICH), south_(Latitude::SOUTH_POLE), east_(Longitude::GLOBE) {}


BoundingBox::BoundingBox(const Latitude& north, const Longitude& west, const Latitude& south, const Longitude& east) :
    north_(north), west_(west), south_(south), east_(east) {
    normalise();
    check(*this);
}


BoundingBox::BoundingBox(const param::MIRParametrisation& param) {

    double box[4];
    ASSERT(param.get("north", box[0]));
    ASSERT(param.get("west", box[1]));
    ASSERT(param.get("south", box[2]));
    ASSERT(param.get("east", box[3]));

    double angularPrecision = 0.;
    param.get("angular_precision", angularPrecision);

    if (angularPrecision > 0.) {

        const eckit::Fraction precision(angularPrecision);
        north_ = eckit::Fraction(box[0], precision);
        west_  = eckit::Fraction(box[1], precision);
        south_ = eckit::Fraction(box[2], precision);
        east_  = eckit::Fraction(box[3], precision);
    }
    else {

        north_ = box[0];
        west_  = box[1];
        south_ = box[2];
        east_  = box[3];
    }

    normalise();
    check(*this);
}


BoundingBox::BoundingBox(const BoundingBox& other) {
    operator=(other);
}


bool BoundingBox::operator==(const BoundingBox& other) const {
    return (north_ == other.north_) && (south_ == other.south_) && (west_ == other.west_) && (east_ == other.east_);
}


BoundingBox& BoundingBox::operator=(const BoundingBox& other) = default;


BoundingBox::~BoundingBox() = default;


void BoundingBox::print(std::ostream& out) const {
    out << "BoundingBox["
        << "north=" << north_ << ",west=" << west_ << ",south=" << south_ << ",east=" << east_ << "]";
}


void BoundingBox::fillGrib(grib_info& info) const {
    // Warning: scanning mode not considered
    info.grid.latitudeOfFirstGridPointInDegrees  = north_.value();
    info.grid.longitudeOfFirstGridPointInDegrees = west_.value();
    info.grid.latitudeOfLastGridPointInDegrees   = south_.value();
    info.grid.longitudeOfLastGridPointInDegrees  = east_.value();

    info.extra_set("expandBoundingBox", 1L);
}


void BoundingBox::hash(eckit::MD5& md5) const {
    md5.add(north_);
    md5.add(west_);
    md5.add(south_);
    md5.add(east_);
}


void BoundingBox::fillJob(api::MIRJob& job) const {
    job.set("area", north_.value(), west_.value(), south_.value(), east_.value());
}


bool BoundingBox::isPeriodicWestEast() const {
    return (west_ != east_) && (west_ == east_.normalise(west_));
}


void BoundingBox::normalise() {
    if (west_ != east_) {
        Longitude eastNormalised = east_.normalise(west_);
        if (eastNormalised == west_) {
            eastNormalised += Longitude::GLOBE;
        }
        east_ = eastNormalised;
    }

    ASSERT(west_ <= east_);
    ASSERT(east_ <= west_ + Longitude::GLOBE);
}


bool BoundingBox::contains(const PointLatLon& p) const {
    return contains(p.lat(), p.lon());
}


bool BoundingBox::contains(const Point2& p) const {
    // notice the order
    return contains(p[0], p[1]);
}


bool BoundingBox::contains(const Latitude& lat, const Longitude& lon) const {
    return (lat <= north_) && (lat >= south_) && (lon.normalise(west_) <= east_);
}


bool BoundingBox::contains(const BoundingBox& other) const {

    if (other.empty()) {
        return contains(other.south(), other.west());
    }

    // check for West/East range (if non-periodic), then other's corners
    if (east_ - west_ < other.east() - other.west() || east_ < other.east().normalise(west_)) {
        return false;
    }

    return contains(other.north(), other.west()) && contains(other.north(), other.east()) &&
           contains(other.south(), other.west()) && contains(other.south(), other.east());
}


bool BoundingBox::intersects(BoundingBox& other) const {

    Latitude n = std::min(north_, other.north_);
    Latitude s = std::max(south_, other.south_);

    bool intersectsSN = s <= n;
    if (!intersectsSN) {
        n = s;
    }

    if (isPeriodicWestEast() && other.isPeriodicWestEast()) {
        other = {n, other.west_, s, other.east_};
        return intersectsSN;
    }

    Longitude w = std::min(west_, other.west_);
    Longitude e = w;

    auto intersect = [](const BoundingBox& a, const BoundingBox& b, Longitude& w, Longitude& e) {
        bool p = a.isPeriodicWestEast();
        if (p || b.isPeriodicWestEast()) {
            w = (p ? b : a).west_;
            e = (p ? b : a).east_;
            return true;
        }

        Longitude ref = b.west_.normalise(a.west_);
        Longitude w_  = std::max(a.west_, ref);
        Longitude e_  = std::min(a.east_, b.east_.normalise(ref));

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
    return !eckit::types::is_strictly_greater(north_.value(), south_.value()) ||
           !eckit::types::is_strictly_greater(east_.value(), west_.value());
}


void BoundingBox::makeName(std::ostream& out) const {
    out << "-" << north_ << ":" << west_ << ":" << south_ << ":" << east_;
}


}  // namespace util
}  // namespace mir
