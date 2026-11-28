// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/gauss/regular/Regular.h"

#include "eckit/geo/range/Regular.h"
#include "eckit/log/JSON.h"
#include "eckit/spec/Custom.h"
#include "eckit/types/FloatCompare.h"

#include "mir/api/MIRJob.h"
#include "mir/api/mir_config.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Angles.h"
#include "mir/util/Atlas.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"


namespace mir::repres::gauss::regular {


Regular::Regular(const param::MIRParametrisation& parametrisation) :
    Gaussian(parametrisation), k_(0), Ni_(0), Nj_(0), order_([&parametrisation]() {
        std::string order;  // (empty for the default)
        parametrisation.get("order", order);
        return grib_order(order);
    }()) {
    // adjust latitudes, longitudes and re-set bounding box
    auto n = bbox_.north();
    auto s = bbox_.south();
    correctSouthNorth(s, n);

    auto e = bbox_.east();
    auto w = bbox_.west();
    correctWestEast(w, e);

    auto old(bbox_);
    bbox_ = util::BoundingBox(n, w, s, e);

    Log::debug() << "Regular::Regular: BoundingBox:"
                 << "\n\t   " << old << "\n\t > " << bbox_ << std::endl;
    setNiNj();
}


Regular::Regular(size_t N, const util::BoundingBox& bbox, double angularPrecision, const std::string& order) :
    Gaussian(N, bbox, angularPrecision), k_(0), Ni_(0), Nj_(0), order_(grib_order(order)) {

    // adjust latitudes, longitudes and re-set bounding box
    auto n = bbox.north();
    auto s = bbox.south();
    correctSouthNorth(s, n);

    auto w = bbox.west();
    auto e = bbox.east();
    correctWestEast(w, e);

    bbox_ = util::BoundingBox(n, w, s, e);

    setNiNj();
}


void Regular::fillGrib(grib_info& info) const {

    // See copy_spec_from_ksec.c in libemos for info

    info.grid.grid_type = CODES_UTIL_GRID_SPEC_REGULAR_GG;

    info.grid.N                            = static_cast<long>(N_);
    info.grid.iDirectionIncrementInDegrees = getSmallestIncrement();
    info.grid.Ni                           = static_cast<long>(Ni_);
    info.grid.Nj                           = static_cast<long>(Nj_);

    bbox_.fillGrib(info);
    grib_fill_scanning_mode(info, order_);
}


void Regular::fillSpec(CustomSpec& spec) const {
    Gaussian::fillSpec(spec);

    spec.set("grid", "F" + std::to_string(N_));

    spec.set("order", order_);
}


void Regular::makeName(std::ostream& out) const {
    out << "F" << N_;
    bbox_.makeName(out);
}


void Regular::correctWestEast(double& w, double& e) const {

    auto eq = [](double a, double b) { return eckit::types::is_approximately_equal(a, b, PointLonLat::EPS); };
    ASSERT(eckit::types::is_approximately_lesser_or_equal(w, e, PointLonLat::EPS));

    const auto inc = getSmallestIncrement();
    ASSERT(inc > 0);

    if (angleApproximatelyEqual(0., w) &&
        (angleApproximatelyEqual(LONGITUDE_GLOBE - inc, e - w) || LONGITUDE_GLOBE - inc < e - w ||
         (!eq(e, w) && eq(util::normalise_longitude(e, w), w)))) {

        w = 0.;
        e = LONGITUDE_GLOBE - inc;
    }
    else {
        // to the longitudes of the grid within the bounds ('inwards'), as encoded
        const auto x = row(static_cast<long>(N_ * 4), w, e, angularPrecision_ > 0 ? angularPrecision_ : 0.);
        ASSERT(x.size > 0);

        w = x.first;
        e = x.last;
    }
}


bool Regular::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const Regular*>(&other);
    return (o != nullptr) && (N_ == o->N_) && (bbox_ == o->bbox_);
}


double Regular::getSmallestIncrement() const {
    ASSERT(N_);
    return PointLonLat::RIGHT_ANGLE / static_cast<double>(N_);
}


size_t Regular::numberOfPoints() const {
    ASSERT(Ni_);
    ASSERT(Nj_);
    return Ni_ * Nj_;
}


bool Regular::getLongestElementDiagonal(double& d) const {
    constexpr double TWO = 2.;

    ASSERT(N_ > 0);
    const auto& lats = latitudes();

    auto snHalf = (lats[N_ - 1] - lats[N_]) / TWO;
    ASSERT(!eckit::types::is_approximately_equal(snHalf, 0.));

    auto weHalf = getSmallestIncrement() / TWO;
    ASSERT(!eckit::types::is_approximately_equal(weHalf, 0.));

    d = TWO * util::Earth::distance(PointLonLat{0., 0.}, PointLonLat{weHalf, snHalf});
    return true;
}


void Regular::json(eckit::JSON& s) const {
    s.startObject();
    s << "type"
      << "regular_gg";
    Gaussian::json(s);
    s.endObject();
}


util::BoundingBox Regular::extendBoundingBox(const util::BoundingBox& bbox) const {

    // adjust West/East to include bbox's West/East (one more longitude of the grid each way, if not around), from the
    // longitudes truncated towards zero
    using eckit::geo::range::Regular;

    const auto inc = getSmallestIncrement();
    auto w         = Regular::adjust(bbox.west(), inc, 0., bbox.west() < 0., PointLonLat::EPS) - inc;
    auto e         = Regular::adjust(bbox.east(), inc, 0., bbox.east() < 0., PointLonLat::EPS) + inc;

    if (std::lround((e - w) / inc) >= static_cast<long>(4 * N_)) {
        w = 0.;
        e = LONGITUDE_GLOBE;
    }

    // adjust South/North to include bbox's South/North ('outwards')
    auto s = bbox.south();
    auto n = bbox.north();
    correctSouthNorth(s, n, false);

    // set bounding box
    const util::BoundingBox extended(n, w, s, e);
    ASSERT(extended.contains(bbox));

    return extended;
}


bool Regular::isPeriodicWestEast() const {
    return eckit::types::is_approximately_greater_or_equal(bbox_.east() - bbox_.west() + getSmallestIncrement(),
                                                           LONGITUDE_GLOBE, PointLonLat::EPS);
}


atlas::Grid Regular::atlasGrid() const {
    return atlas::RegularGaussianGrid("F" + std::to_string(N_), domain());
}


void Regular::setNiNj() {
    ASSERT(N_);

    const auto inc   = getSmallestIncrement();
    const auto& lats = latitudes();

    const auto west  = bbox_.west();
    const auto east  = bbox_.east();
    const auto south = bbox_.south();
    const auto north = bbox_.north();

    constexpr auto EPS = PointLonLat::EPS;

    Ni_ = N_ * 4;

    if (eckit::types::is_strictly_greater(LONGITUDE_GLOBE, east - west + inc, EPS)) {
        Ni_ = row(static_cast<long>(N_ * 4), west, east).size;

        ASSERT(2 <= Ni_ && Ni_ <= N_ * 4);
    }

    k_  = 0;
    Nj_ = N_ * 2;

    if (eckit::types::is_strictly_greater(lats.front(), north, EPS) ||
        eckit::types::is_strictly_greater(south, lats.back(), EPS)) {
        Nj_ = 0;
        for (const auto lat : lats) {
            if (eckit::types::is_strictly_greater(lat, north, EPS) && !angleApproximatelyEqual(north, lat)) {
                ++k_;
            }
            else if (eckit::types::is_approximately_lesser_or_equal(south, lat, EPS) ||
                     angleApproximatelyEqual(south, lat)) {
                ++Nj_;
            }
            else {
                break;
            }
        }
        ASSERT(Nj_ > 0);
    }

    Log::debug() << "Regular::setNiNj: Ni*Nj = " << Ni_ << " * " << Nj_ << " = " << (Ni_ * Nj_) << std::endl;
}


size_t Regular::frame(MIRValuesVector& values, size_t size, double missingValue) const {

    // TODO: Check if that logic cannot also be used for other grid, and therefore move it to a higher class
    validate(values);

    size_t count = 0;

    size_t k = 0;
    for (size_t j = 0; j < Nj_; j++) {
        for (size_t i = 0; i < Ni_; i++) {
            if (!((i < size) || (j < size) || (i >= Ni_ - size) || (j >= Nj_ - size))) {  // Check me, may be buggy
                values[k] = missingValue;
                count++;
            }
            k++;
        }
    }

    ASSERT(k == values.size());
    return count;
}


}  // namespace mir::repres::gauss::regular
