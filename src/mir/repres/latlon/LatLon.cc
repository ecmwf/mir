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
#include "eckit/types/Fraction.h"

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
    reference_(bbox_.south(), bbox_.west()),
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


LatLon::LatLon(const util::Increments& increments, const util::BoundingBox& bbox, const PointLatLon& reference,
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
    const auto inc = increments_.west_east().longitude().fraction();
    return eckit::Fraction(bbox_.east()) - eckit::Fraction(bbox_.west()) + inc >= Longitude::GLOBE.fraction();
}


bool LatLon::includesNorthPole() const {

    // if North latitude is within one increment from North Pole
    const Latitude& inc = increments_.south_north().latitude();
    return bbox_.north() + inc > Latitude::NORTH_POLE;
}


bool LatLon::includesSouthPole() const {

    // if South latitude is within one increment from South Pole
    const Latitude& inc = increments_.south_north().latitude();
    return bbox_.south() - inc < Latitude::SOUTH_POLE;
}


size_t LatLon::numberOfPoints() const {
    ASSERT(ni_);
    ASSERT(nj_);
    return ni_ * nj_;
}


bool LatLon::getLongestElementDiagonal(double& d) const {
    constexpr double TWO = 2.;

    auto snHalf = increments_.south_north().latitude().value() / TWO;
    ASSERT(!eckit::types::is_approximately_equal(snHalf, 0.));

    auto weHalf = increments_.west_east().longitude().value() / TWO;
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
    globaliseBoundingBox(newbbox, increments_, {bbox_.south(), bbox_.west()});

    std::unique_ptr<LatLon> newll(const_cast<LatLon*>(croppedRepresentation(newbbox)));

    ASSERT(newll->nj_ > nj_);
    ASSERT(newll->ni_ == ni_);

    size_t n    = ni_ * nj_;
    size_t newn = newll->ni_ * newll->nj_;

    Latitude lat   = newbbox.north();
    Longitude lon  = newbbox.west();
    size_t nBefore = 0;
    while (!bbox_.contains(lat, lon)) {
        lat -= increments_.south_north().latitude();
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


void LatLon::globaliseBoundingBox(util::BoundingBox& bbox, const util::Increments& inc, const PointLatLon& reference) {
    using eckit::geo::range::Regular;

    const auto sn = inc.south_north().latitude().fraction();
    const auto we = inc.west_east().longitude().fraction();
    ASSERT(sn > 0);
    ASSERT(we > 0);


    // Latitude limits (of the reference as encoded, see correctBoundingBox)

    Latitude n = Regular::adjust(Latitude::NORTH_POLE.fraction(), sn, reference.lat().fraction(), false, PRECISION);
    Latitude s = Regular::adjust(Latitude::SOUTH_POLE.fraction(), sn, reference.lat().fraction(), true, PRECISION);


    // Longitude limits
    // - West for non-periodic grids is not corrected!
    // - East for periodic grids is W + 360 - increment

    Longitude w = bbox.west();
    if (inc.isPeriodic()) {
        w = Regular::adjust(Longitude::GREENWICH.fraction(), we, reference.lon().fraction(), true, PRECISION);
    }

    Longitude e =
        Regular::adjust(w.fraction() + Longitude::GLOBE.fraction(), we, reference.lon().fraction(), false, PRECISION);
    if (e - w == Longitude::GLOBE) {
        e -= we;
    }


    // set bounding box
    bbox = {n, w, s, e};
}


void LatLon::correctBoundingBox(util::BoundingBox& bbox, size_t& ni, size_t& nj, const util::Increments& inc,
                                const PointLatLon& reference) {
    using eckit::geo::range::Regular;

    // bounds within the precision of a grid point include it, as encoded to the microdegree (MIR-351)
    auto bound = [](const eckit::Fraction& value, const eckit::Fraction& inc, const eckit::Fraction& ref, bool up) {
        return inc == 0 ? value : Regular::adjust(value, inc, ref, up, PRECISION);
    };

    // Latitude/longitude ranges
    const auto sn = inc.south_north().latitude().fraction();
    const eckit::geo::range::RegularLatitude lat(
        sn, bound(eckit::Fraction(bbox.south()), sn, reference.lat().fraction(), true),
        bound(eckit::Fraction(bbox.north()), sn, reference.lat().fraction(), false), reference.lat().fraction());
    auto n = lat.bf();
    auto s = lat.af();

    nj = lat.size();
    ASSERT(nj > 0);

    const auto we = inc.west_east().longitude().fraction();
    const eckit::geo::range::RegularLongitude lon(
        we, bound(eckit::Fraction(bbox.west()), we, reference.lon().fraction(), true),
        bound(eckit::Fraction(bbox.east()), we, reference.lon().fraction(), false), reference.lon().fraction());
    auto w = lon.af();
    auto e = lon.bf();

    ni = lon.size();
    ASSERT(ni > 0);

    // checks
    ASSERT(w + (ni - 1) * lon.increment() == e || ni * lon.increment() == Longitude::GLOBE.fraction());
    ASSERT(s + (nj - 1) * lat.increment() == n);

    bbox = {double(n), double(w), double(s), double(e)};
}


}  // namespace mir::repres::latlon
