// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/latlon/LatLon.h"

#include <memory>
#include <ostream>
#include <sstream>

#include "eckit/geo/range/Regular.h"
#include "eckit/log/JSON.h"
#include "eckit/spec/Custom.h"
#include "eckit/types/FloatCompare.h"

#include "mir/data/MIRField.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"
#include "mir/util/MeshGeneratorParameters.h"
#include "mir/util/Types.h"


namespace mir::repres::latlon {


LatLon::LatLon(const param::MIRParametrisation& parametrisation) :
    Gridded(parametrisation),
    increments_(parametrisation),
    reference_(bbox_.west(), bbox_.south()),
    ni_(0),
    nj_(0),
    order_([&parametrisation]() {
        std::string order;  // (empty for the default)
        parametrisation.get("order", order);
        return grib_order(order);
    }()) {
    correctBoundingBox(bbox_, ni_, nj_, increments_, reference_);
    ASSERT(ni_ != 0);
    ASSERT(nj_ != 0);

    // confirm Ni/Nj from parametrisation (input)
    size_t ni = 0;
    size_t nj = 0;
    ASSERT(parametrisation.get("Ni", ni));
    ASSERT(parametrisation.get("Nj", nj));

    Log::debug() << "LatLon:"
                    "\n\t"
                    "(Ni, Nj) = ("
                 << ni_ << ", " << nj_
                 << ") calculated"
                    "\n\t"
                    "(Ni, Nj) = ("
                 << ni << ", " << nj << ") from parametrisation" << std::endl;

    ASSERT(ni == ni_);
    ASSERT(nj == nj_);
}


LatLon::LatLon(const util::Increments& increments, const util::BoundingBox& bbox, const PointLonLat& reference,
               const std::string& order) :
    Gridded(bbox), increments_(increments), reference_(reference), ni_(0), nj_(0), order_(grib_order(order)) {
    correctBoundingBox(bbox_, ni_, nj_, increments_, reference_);
    ASSERT(ni_ != 0);
    ASSERT(nj_ != 0);
}


LatLon::~LatLon() = default;


void LatLon::print(std::ostream& out) const {
    out << "LatLon["
        << "bbox=" << bbox_ << ",increments=" << increments_ << ",ni=" << ni_ << ",nj=" << nj_ << "]";
}


void LatLon::json(eckit::JSON& s) const {
    s << "bbox" << bbox_;
    s << "increments" << increments_;
    s << "ni" << ni_;
    s << "nj" << nj_;
}


void LatLon::fillGrib(grib_info& info) const {
    // See copy_spec_from_ksec.c in libemos for info
    // Warning: scanning mode not considered

    info.grid.Ni = static_cast<long>(ni_);
    info.grid.Nj = static_cast<long>(nj_);

    increments_.fillGrib(info);
    bbox_.fillGrib(info);
    grib_fill_scanning_mode(info, order_);
}


void LatLon::fillSpec(CustomSpec& spec) const {
    Gridded::fillSpec(spec);
    increments_.fillSpec(spec);
    spec.set("order", order_);
}


void LatLon::makeName(std::ostream& out) const {
    out << "LL";
    increments_.makeName(out);
    bbox_.makeName(out);
}


bool LatLon::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const LatLon*>(&other);
    return (o != nullptr) && (bbox_ == o->bbox_) && (increments_ == o->increments_);
}


bool LatLon::isPeriodicWestEast() const {

    // if range West-East is within one increment (or greater than) 360 degree
    return eckit::types::is_approximately_greater_or_equal(bbox_.east() - bbox_.west() + increments_.west_east(),
                                                           LONGITUDE_GLOBE, PointLonLat::EPS);
}


bool LatLon::includesNorthPole() const {

    // if North latitude is within one increment from North Pole
    return eckit::types::is_strictly_greater(bbox_.north() + increments_.south_north(), NORTH_POLE, PointLonLat::EPS);
}


bool LatLon::includesSouthPole() const {

    // if South latitude is within one increment from South Pole
    return eckit::types::is_strictly_greater(SOUTH_POLE, bbox_.south() - increments_.south_north(), PointLonLat::EPS);
}


size_t LatLon::numberOfPoints() const {
    ASSERT(ni_);
    ASSERT(nj_);
    return ni_ * nj_;
}


bool LatLon::getLongestElementDiagonal(double& d) const {
    constexpr double TWO = 2.;

    auto snHalf = increments_.south_north() / TWO;
    ASSERT(!eckit::types::is_approximately_equal(snHalf, 0.));

    auto weHalf = increments_.west_east() / TWO;
    ASSERT(!eckit::types::is_approximately_equal(weHalf, 0.));

    d = TWO * util::Earth::distance(PointLonLat{0., 0.}, PointLonLat{weHalf, snHalf});
    return true;
}


Representation* LatLon::globalise(data::MIRField& field) const {
    ASSERT(field.representation() == this);

    if (isGlobal()) {
        return nullptr;
    }

    // For now, we only grow towards North and South poles
    ASSERT(domain().isPeriodicWestEast());

    util::BoundingBox newbbox(bbox_);
    globaliseBoundingBox(newbbox, increments_, {bbox_.west(), bbox_.south()});

    std::unique_ptr<LatLon> newll(const_cast<LatLon*>(croppedRepresentation(newbbox)));

    ASSERT(newll->nj_ > nj_);
    ASSERT(newll->ni_ == ni_);

    size_t n    = ni_ * nj_;
    size_t newn = newll->ni_ * newll->nj_;

    auto lat       = newbbox.north();
    const auto lon = newbbox.west();
    size_t nBefore = 0;
    while (!bbox_.contains(lat, lon)) {
        lat -= increments_.south_north();
        nBefore += ni_;
        ASSERT(n + nBefore <= newn);
    }

    for (size_t i = 0; i < field.dimensions(); i++) {
        MIRValuesVector newvalues(newn, field.missingValue());
        const MIRValuesVector& values = field.direct(i);
        ASSERT(values.size() == n);

        for (size_t j = 0; j < n; ++j) {
            newvalues[j + nBefore] = values[j];
        }

        field.update(newvalues, i);
    }

    field.hasMissing(true);

    return newll.release();
}


const LatLon* LatLon::croppedRepresentation(const util::BoundingBox& /*unused*/) const {
    std::ostringstream os;
    os << "LatLon::croppedRepresentation() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


void LatLon::fillMeshGen(util::MeshGeneratorParameters& params) const {
    if (params.meshGenerator_.empty()) {
        params.meshGenerator_ = "structured";
    }
    if (eckit::types::is_strictly_greater(boundingBox().south(), 0., PointLonLat::EPS)) {
        params.set("force_include_south_pole", true);
    }
    if (eckit::types::is_strictly_greater(0., boundingBox().north(), PointLonLat::EPS)) {
        params.set("force_include_north_pole", true);
    }
}


size_t LatLon::frame(MIRValuesVector& values, size_t size, double missingValue) const {

    // Could be done better, just a demo
    validate(values);

    size_t count = 0;

    size_t k = 0;
    for (size_t j = 0; j < nj_; j++) {
        for (size_t i = 0; i < ni_; i++) {
            if (!((i < size) || (j < size) || (i >= ni_ - size) || (j >= nj_ - size))) {  // Check me, may be buggy
                values[k] = missingValue;
                count++;
            }
            k++;
        }
    }

    // Log::info() << "LatLon::frame(" << size << ") " << count << " " << k << std::endl;


    ASSERT(k == values.size());
    return count;
}


void LatLon::validate(const MIRValuesVector& values) const {
    const size_t count = numberOfPoints();

    Log::debug() << "LatLon::validate checked " << Log::Pretty(values.size(), {"value"}) << ", iterator counts "
                 << Log::Pretty(count) << " (" << domain() << ")." << std::endl;

    ASSERT_VALUES_SIZE_EQ_ITERATOR_COUNT("LatLon", values.size(), count);
}


void LatLon::globaliseBoundingBox(util::BoundingBox& bbox, const util::Increments& inc, const PointLonLat& reference) {
    using eckit::geo::range::Regular;

    const auto sn = inc.south_north();
    const auto we = inc.west_east();
    ASSERT(sn > 0);
    ASSERT(we > 0);


    // Latitude limits (of the reference as encoded, see correctBoundingBox)

    const auto n = Regular::adjust(NORTH_POLE, sn, reference.lat(), false, PRECISION);
    const auto s = Regular::adjust(SOUTH_POLE, sn, reference.lat(), true, PRECISION);


    // Longitude limits
    // - West for non-periodic grids is not corrected!
    // - East for periodic grids is W + 360 - increment

    auto w = bbox.west();
    if (inc.isPeriodic()) {
        w = Regular::adjust(0., we, reference.lon(), true, PRECISION);
    }

    auto e = Regular::adjust(w + LONGITUDE_GLOBE, we, reference.lon(), false, PRECISION);
    if (eckit::types::is_approximately_equal(e - w, LONGITUDE_GLOBE, PointLonLat::EPS)) {
        e -= we;
    }


    // set bounding box
    bbox = {n, w, s, e};
}


void LatLon::correctBoundingBox(util::BoundingBox& bbox, size_t& ni, size_t& nj, const util::Increments& inc,
                                const PointLonLat& reference) {
    using eckit::geo::range::Regular;

    // bounds within the precision of a grid point include it, as encoded to the microdegree (MIR-351)
    auto bound = [](double value, double inc, double ref, bool up) {
        return inc == 0 ? value : Regular::adjust(value, inc, ref, up, PRECISION);
    };

    auto eq = [](double a, double b) { return eckit::types::is_approximately_equal(a, b, PointLonLat::EPS); };

    // Latitude/longitude ranges
    const auto sn = inc.south_north();
    const eckit::geo::range::RegularLatitude lat(sn, bound(bbox.south(), sn, reference.lat(), true),
                                                 bound(bbox.north(), sn, reference.lat(), false), reference.lat());
    const auto n = lat.b();
    const auto s = lat.a();

    nj = lat.size();
    ASSERT(nj > 0);

    const auto we = inc.west_east();
    const eckit::geo::range::RegularLongitude lon(we, bound(bbox.west(), we, reference.lon(), true),
                                                  bound(bbox.east(), we, reference.lon(), false), reference.lon());
    const auto w = lon.a();
    const auto e = lon.b();

    ni = lon.size();
    ASSERT(ni > 0);

    // checks, of the ranges' increments (as fractions, which the increments approximate)
    const auto lon_inc = static_cast<double>(lon.increment());
    const auto lat_inc = static_cast<double>(lat.increment());
    ASSERT(eq(w + static_cast<double>(ni - 1) * lon_inc, e) || eq(static_cast<double>(ni) * lon_inc, LONGITUDE_GLOBE));
    ASSERT(eq(s + static_cast<double>(nj - 1) * lat_inc, n));

    bbox = {n, w, s, e};
}


}  // namespace mir::repres::latlon
