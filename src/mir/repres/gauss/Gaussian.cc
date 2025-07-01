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


#include "mir/repres/gauss/Gaussian.h"

#include <algorithm>
#include <cmath>
#include <map>

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


bool Gaussian::extendBoundingBoxOnIntersect() const {
    return false;
}


bool Gaussian::angleApproximatelyEqual(double a, double b) const {
    return angularPrecision_ > 0 ? eckit::types::is_approximately_equal(a, b, angularPrecision_) : a == b;
}


void Gaussian::correctSouthNorth(double& s, double& n, bool in) const {
    ASSERT(s <= n);

    const std::vector<double>& lats = latitudes();
    ASSERT(!lats.empty());

    const bool same(s == n);
    if (n < lats.back()) {
        n = lats.back();
    }
    else if (in) {
        auto best = std::lower_bound(lats.begin(), lats.end(), n, [this](double l1, double l2) {
            if (angleApproximatelyEqual(l1, l2)) {
                return false;
            }
            return !(l1 < l2);
        });
        ASSERT(best != lats.end());
        n = *best;
    }
    else if (n > lats.front()) {
        // extend 'outwards': don't change, it's already above the Gaussian latitudes
    }
    else {
        auto best = std::lower_bound(lats.rbegin(), lats.rend(), n);
        n         = *best;
    }

    if (same && in) {
        s = n;
    }
    else if (s > lats.front()) {
        s = lats.front();
    }
    else if (in) {
        auto best = std::lower_bound(lats.rbegin(), lats.rend(), s, [this](double l1, double l2) {
            if (angleApproximatelyEqual(l1, l2)) {
                return false;
            }
            return !(l1 > l2);
        });
        ASSERT(best != lats.rend());
        s = *best;
    }
    else if (s < lats.back()) {
        // extend 'outwards': don't change, it's already below the Gaussian latitudes
    }
    else {
        auto best = std::lower_bound(lats.begin(), lats.end(), s, [](double l1, double l2) { return l1 > l2; });
        s         = *best;
    }

    ASSERT(s <= n);
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

    *(f++) = PointLonLat::RIGHT_ANGLE;
    *(b++) = -PointLonLat::RIGHT_ANGLE;

    double wacc = -1.;
    for (size_t j = 0; j < N_; ++j, ++b, ++f) {
        wacc += 2. * w[j];
        double deg = util::radian_to_degree(std::asin(wacc));
        ASSERT(-PointLonLat::RIGHT_ANGLE <= deg && deg <= PointLonLat::RIGHT_ANGLE);

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

    if (auto s = bbox_.south(); s <= latitudes().back() || s > 0) {
        params.set("force_include_south_pole", true);
    }

    if (auto n = bbox_.north(); n >= latitudes().front() || n < 0) {
        params.set("force_include_north_pole", true);
    }
}


void Gaussian::fillJob(api::MIRJob& job) const {
    auto d = domain();
    if (!d.isGlobal() || d.west() != 0.) {
        bbox_.fillJob(job);
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
