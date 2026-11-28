// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/util/BoundingBox.h"
#include "mir/util/Angles.h"

#include <algorithm>
#include <ostream>

#include "eckit/geo/area/BoundingBox.h"
#include "eckit/log/JSON.h"
#include "eckit/spec/Custom.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/api/MIRJob.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"


namespace mir::util {


static eckit::geo::area::BoundingBox GLOBAL;


namespace {


constexpr auto EPS = PointLonLat::EPS;


bool eq(double a, double b) {
    return eckit::types::is_approximately_equal(a, b, EPS);
}


bool le(double a, double b) {
    return eckit::types::is_approximately_lesser_or_equal(a, b, EPS);
}


bool lt(double a, double b) {
    return eckit::types::is_strictly_greater(b, a, EPS);
}


double get(const param::MIRParametrisation& param, const char* key, double dfault) {
    double value = dfault;
    param.get(key, value);
    return value;
}


}  // namespace


BoundingBox::BoundingBox() : north_(NORTH_POLE), west_(0.), south_(SOUTH_POLE), east_(LONGITUDE_GLOBE) {}


BoundingBox::BoundingBox(double north, double west, double south, double east) :
    north_(north), west_(west), south_(south), east_(east) {
    if (!eq(west_, east_)) {
        auto eastNormalised = normalise_longitude(east_, west_);
        if (eq(eastNormalised, west_)) {
            eastNormalised += LONGITUDE_GLOBE;
        }
        east_ = eastNormalised;
    }

    ASSERT(le(west_, east_) && le(east_, west_ + LONGITUDE_GLOBE));
    ASSERT(le(SOUTH_POLE, south_) && le(south_, north_) && le(north_, NORTH_POLE));
}


BoundingBox::BoundingBox(const param::MIRParametrisation& param) :
    BoundingBox(get(param, "north", GLOBAL.north()), get(param, "west", GLOBAL.west()),
                get(param, "south", GLOBAL.south()), get(param, "east", GLOBAL.east())) {}


BoundingBox::BoundingBox(const BoundingBox&) = default;


bool BoundingBox::operator==(const BoundingBox& other) const {
    return eq(north_, other.north_) && eq(south_, other.south_) && eq(west_, other.west_) && eq(east_, other.east_);
}


BoundingBox& BoundingBox::operator=(const BoundingBox& other) = default;


BoundingBox::~BoundingBox() = default;


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


void BoundingBox::fillSpec(eckit::spec::Custom& spec) const {
    spec.set("area", std::vector<double>{north_, west_, south_, east_});
}


bool BoundingBox::isPeriodicWestEast() const {
    // the east is normalised to the west (in ]west, west + 360])
    return !eq(west_, east_) && eq(east_ - west_, LONGITUDE_GLOBE);
}


bool BoundingBox::contains(const PointLonLat& p) const {
    return contains(p.lat(), p.lon());
}


bool BoundingBox::contains(double lat, double lon) const {
    return le(lat, north_) && le(south_, lat) && le(normalise_longitude(lon, west_), east_);
}


bool BoundingBox::contains(const BoundingBox& other) const {
    if (other.empty()) {
        return contains(other.south(), other.west());
    }

    // check for West/East range (if non-periodic), then other's corners
    if (lt(east_ - west_, other.east() - other.west()) || lt(east_, normalise_longitude(other.east(), west_))) {
        return false;
    }

    return contains(other.north(), other.west()) && contains(other.north(), other.east()) &&
           contains(other.south(), other.west()) && contains(other.south(), other.east());
}


bool BoundingBox::intersects(BoundingBox& other) const {
    auto n = std::min(north_, other.north_);
    auto s = std::max(south_, other.south_);

    bool intersectsSN = le(s, n);
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

        auto ref = normalise_longitude(b.west_, a.west_);
        auto w_  = std::max(a.west_, ref);
        auto e_  = std::min(a.east_, normalise_longitude(b.east_, ref));

        if (le(w_, e_)) {
            w = w_;
            e = e_;
            return true;
        }
        return false;
    };

    bool intersectsWE = le(west_, other.west_) ? intersect(*this, other, w, e) || intersect(other, *this, w, e)
                                               : intersect(other, *this, w, e) || intersect(*this, other, w, e);

    ASSERT(le(w, e));
    other = {n, w, s, e};

    return intersectsSN && intersectsWE;
}


bool BoundingBox::empty() const {
    return !eckit::types::is_strictly_greater(north_, south_, EPS) ||
           !eckit::types::is_strictly_greater(east_, west_, EPS);
}


void BoundingBox::makeName(std::ostream& out) const {
    out << "-" << north_ << ":" << west_ << ":" << south_ << ":" << east_;
}

void BoundingBox::json(eckit::JSON& json) const {
    json.startObject();
    json << "north" << north_;
    json << "west" << west_;
    json << "south" << south_;
    json << "east" << east_;
    json.endObject();
}

}  // namespace mir::util
