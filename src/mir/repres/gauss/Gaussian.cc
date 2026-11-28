// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/repres/gauss/Gaussian.h"

#include <algorithm>
#include <cmath>
#include <map>

#include "eckit/geo/Range.h"
#include "eckit/geo/range/Regular.h"
#include "eckit/geo/util.h"
#include "eckit/log/JSON.h"
#include "eckit/types/FloatCompare.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Angles.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/MeshGeneratorParameters.h"
#include "mir/util/Mutex.h"
#include "mir/util/Trace.h"
#include "mir/util/Types.h"


namespace mir::repres {


static util::once_flag once;
static util::recursive_mutex* local_mutex         = nullptr;
static std::map<size_t, std::vector<double> >* ml = nullptr;
static std::map<size_t, std::vector<double> >* mw = nullptr;
static void init() {
    local_mutex = new util::recursive_mutex();
    ml          = new std::map<size_t, std::vector<double> >();
    mw          = new std::map<size_t, std::vector<double> >();
}


Gaussian::Gaussian(size_t N, const util::BoundingBox& bbox, double angularPrecision) :
    Gridded(bbox), N_(N), angularPrecision_(angularPrecision) {
    ASSERT(N_ > 0);
    ASSERT(angularPrecision >= 0);
}


Gaussian::Gaussian(const param::MIRParametrisation& parametrisation) :
    Gridded(parametrisation), N_(0), angularPrecision_(0) {

    ASSERT(parametrisation.get("N", N_));
    ASSERT(N_ > 0);

    parametrisation.get("angular_precision", angularPrecision_);
    ASSERT(angularPrecision_ >= 0);
}


Gaussian::~Gaussian() = default;


bool Gaussian::sameAs(const Representation& other) const {
    const auto* o = dynamic_cast<const Gaussian*>(&other);
    return (o != nullptr) && (N_ == o->N_) && (domain() == o->domain());
}


bool Gaussian::includesNorthPole() const {
    return bbox_.north() >= latitudes().front();
}


bool Gaussian::includesSouthPole() const {
    return bbox_.south() <= latitudes().back();
}


void Gaussian::validate(const MIRValuesVector& values) const {
    const size_t count = numberOfPoints();

    Log::debug() << "Gaussian::validate checked " << Log::Pretty(values.size(), {"value"}) << ", iterator counts "
                 << Log::Pretty(count) << " (" << domain() << ")." << std::endl;

    ASSERT_VALUES_SIZE_EQ_ITERATOR_COUNT("Gaussian", values.size(), count);
}


bool Gaussian::angleApproximatelyEqual(double A, double B) const {
    return eckit::types::is_approximately_equal(A, B, angularPrecision_ > 0 ? angularPrecision_ : PointLonLat::EPS);
}


void Gaussian::correctSouthNorth(double& s, double& n, bool in) const {
    auto lt = [](double a, double b) { return eckit::types::is_strictly_greater(b, a, PointLonLat::EPS); };
    ASSERT(!lt(n, s));

    const std::vector<double>& lats = latitudes();
    ASSERT(!lats.empty());

    const bool same = eckit::types::is_approximately_equal(s, n, PointLonLat::EPS);
    if (lt(n, lats.back())) {
        n = lats.back();
    }
    else if (in) {
        auto best = std::lower_bound(lats.begin(), lats.end(), n, [this, &lt](double l1, double l2) {
            if (angleApproximatelyEqual(l1, l2)) {
                return false;
            }
            return !lt(l1, l2);
        });
        ASSERT(best != lats.end());
        n = *best;
    }
    else if (lt(lats.front(), n)) {
        // extend 'outwards': don't change, it's already above the Gaussian latitudes
    }
    else {
        auto best = std::lower_bound(lats.rbegin(), lats.rend(), n);
        n         = *best;
    }

    if (same && in) {
        s = n;
    }
    else if (lt(lats.front(), s)) {
        s = lats.front();
    }
    else if (in) {
        auto best = std::lower_bound(lats.rbegin(), lats.rend(), s, [this, &lt](double l1, double l2) {
            if (angleApproximatelyEqual(l1, l2)) {
                return false;
            }
            return !lt(l2, l1);
        });
        ASSERT(best != lats.rend());
        s = *best;
    }
    else if (lt(s, lats.back())) {
        // extend 'outwards': don't change, it's already below the Gaussian latitudes
    }
    else {
        auto best = std::lower_bound(lats.begin(), lats.end(), s, [](double l1, double l2) { return l1 > l2; });
        s         = *best;
    }

    ASSERT(!lt(n, s));
}


Gaussian::Row Gaussian::row(long Ni, double west, double east, double precision) {
    ASSERT(Ni > 0);
    ASSERT(precision >= 0.);
    const eckit::geo::range::RegularLongitude global(LONGITUDE_GLOBE / static_cast<double>(Ni), 0., LONGITUDE_GLOBE);
    const std::unique_ptr<eckit::geo::Range> x(global.make_cropped_range(west - precision, east + precision));

    // the first and last longitudes (from West), as k * 360 / Ni rounded once (as the grid's, fractions)
    const auto inc  = LONGITUDE_GLOBE / static_cast<double>(Ni);
    const auto size = x->size();
    if (size == 0) {
        return {west, west, inc, 0};
    }

    const auto k     = std::lround(util::normalise_longitude(x->a(), west - precision) / inc);
    const auto first = static_cast<double>(k) * LONGITUDE_GLOBE / static_cast<double>(Ni);
    const auto last  = static_cast<double>(k + static_cast<long>(size) - 1) * LONGITUDE_GLOBE / static_cast<double>(Ni);
    return {first, last, inc, size};
}


std::vector<double> Gaussian::calculateUnrotatedGridBoxLatitudeEdges() const {

    // grid-box edge latitudes are the accumulated Gaussian quadrature weights
    size_t Nj = N_ * 2;
    ASSERT(Nj > 1);

    const auto& w = weights();
    ASSERT(w.size() == Nj);

    std::vector<double> edges(Nj + 1);
    auto f = edges.begin();
    auto b = edges.rbegin();

    *(f++) = NORTH_POLE;
    *(b++) = SOUTH_POLE;

    double wacc = -1.;
    for (size_t j = 0; j < N_; ++j, ++b, ++f) {
        wacc += 2. * w[j];
        double deg = util::radian_to_degree(std::asin(wacc));
        ASSERT(SOUTH_POLE <= deg && deg <= NORTH_POLE);

        *b = deg;
        *f = -(*b);
    }

    edges[N_] = 0.;  // exact value

    return edges;
}


void Gaussian::fillMeshGen(util::MeshGeneratorParameters& params) const {
    if (params.meshGenerator_.empty()) {
        params.meshGenerator_ = "structured";
    }

    constexpr auto EPS = PointLonLat::EPS;

    const auto s = bbox_.south();
    if (eckit::types::is_approximately_lesser_or_equal(s, latitudes().back(), EPS) ||
        eckit::types::is_strictly_greater(s, 0., EPS)) {
        params.set("force_include_south_pole", true);
    }

    const auto n = bbox_.north();
    if (eckit::types::is_approximately_greater_or_equal(n, latitudes().front(), EPS) ||
        eckit::types::is_strictly_greater(0., n, EPS)) {
        params.set("force_include_north_pole", true);
    }
}


const std::vector<double>& Gaussian::latitudes(size_t N) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    ASSERT(N);
    auto j = ml->find(N);
    if (j == ml->end()) {
        trace::Timer timer("Gaussian latitudes " + std::to_string(N));
        (*ml)[N] = eckit::geo::util::gaussian_latitudes(N, false);
        j        = ml->find(N);
    }
    ASSERT(j != ml->end());


    // these are the assumptions we expect from the Gaussian latitudes values
    auto& lats = j->second;
    ASSERT(2 * N == lats.size());
    ASSERT(std::is_sorted(lats.begin(), lats.end(), [](double a, double b) { return a > b; }));

    return lats;
}


const std::vector<double>& Gaussian::weights(size_t N) {
    util::call_once(once, init);
    util::lock_guard<util::recursive_mutex> lock(*local_mutex);

    ASSERT(N);
    auto j = mw->find(N);
    if (j == mw->end()) {
        trace::Timer timer("Gaussian quadrature weights " + std::to_string(N));
        (*mw)[N] = eckit::geo::util::gaussian_quadrature_weights(N);
        j        = mw->find(N);
    }
    ASSERT(j != mw->end());
    ASSERT(j->second.size() == 2 * N);

    return j->second;
}


const std::vector<double>& Gaussian::latitudes() const {
    return latitudes(N_);
}


const std::vector<double>& Gaussian::weights() const {
    return weights(N_);
}


void Gaussian::json(eckit::JSON& s) const {
    s << "N" << N_;
    s << "bbox" << bbox_;
    s << "angular_precision" << angularPrecision_;
}


}  // namespace mir::repres
