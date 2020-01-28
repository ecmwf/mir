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

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#include "eckit/types/FloatCompare.h"

#include "mir/api/Atlas.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Angles.h"
#include "mir/util/Domain.h"
#include "mir/util/MeshGeneratorParameters.h"
#include "mir/util/Pretty.h"


namespace mir {
namespace repres {


namespace {
static pthread_once_t once                        = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex                  = nullptr;
static std::map<size_t, std::vector<double> >* ml = nullptr;
static std::map<size_t, std::vector<double> >* mw = nullptr;
static void init() {
    local_mutex = new eckit::Mutex();
    ml          = new std::map<size_t, std::vector<double> >();
    mw          = new std::map<size_t, std::vector<double> >();
}
}  // namespace


Gaussian::Gaussian(size_t N, const util::BoundingBox& bbox, double angularPrecision) :
    Gridded(bbox),
    N_(N),
    angularPrecision_(angularPrecision) {
    ASSERT(N_ > 0);
    ASSERT(angularPrecision >= 0);
}


Gaussian::Gaussian(const param::MIRParametrisation& parametrisation) :
    Gridded(parametrisation),
    N_(0),
    angularPrecision_(0) {

    ASSERT(parametrisation.get("N", N_));
    ASSERT(N_ > 0);

    parametrisation.get("angularPrecision", angularPrecision_);
    ASSERT(angularPrecision_ >= 0);
}


Gaussian::~Gaussian() = default;


bool Gaussian::sameAs(const Representation& other) const {
    auto o = dynamic_cast<const Gaussian*>(&other);
    return o && (N_ == o->N_) && (domain() == o->domain());
}


Iterator* Gaussian::unrotatedIterator(gauss::GaussianIterator::ni_type Ni) const {
    return new gauss::GaussianIterator(latitudes(), bbox_, N_, Ni);
}


Iterator* Gaussian::rotatedIterator(gauss::GaussianIterator::ni_type Ni, const util::Rotation& rotation) const {
    return new gauss::GaussianIterator(latitudes(), bbox_, N_, Ni, rotation);
}


bool Gaussian::includesNorthPole() const {
    return bbox_.north() >= latitudes().front();
}


bool Gaussian::includesSouthPole() const {
    return bbox_.south() <= latitudes().back();
}


void Gaussian::validate(const MIRValuesVector& values) const {
    const size_t count = numberOfPoints();

    eckit::Log::debug<LibMir>() << "Gaussian::validate checked " << Pretty(values.size(), {"value"})
                                << ", within domain: " << Pretty(count) << "." << std::endl;
    ASSERT(values.size() == count);
}


bool Gaussian::extendBoundingBoxOnIntersect() const {
    return false;
}


bool Gaussian::angleApproximatelyEqual(const Latitude& A, const Latitude& B) const {
    return angularPrecision_ > 0 ? eckit::types::is_approximately_equal(A.value(), B.value(), angularPrecision_)
                                 : A == B;
}


bool Gaussian::angleApproximatelyEqual(const Longitude& A, const Longitude& B) const {
    return angularPrecision_ > 0 ? eckit::types::is_approximately_equal(A.value(), B.value(), angularPrecision_)
                                 : A == B;
}


void Gaussian::correctSouthNorth(Latitude& s, Latitude& n, bool in) const {
    ASSERT(s <= n);

    const std::vector<double>& lats = latitudes();
    ASSERT(!lats.empty());

    const bool same(s == n);
    if (n < lats.back()) {
        n = lats.back();
    }
    else if (in) {
        auto best = std::lower_bound(lats.begin(), lats.end(), n, [this](Latitude l1, Latitude l2) {
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
        auto best = std::lower_bound(lats.rbegin(), lats.rend(), s, [this](Latitude l1, Latitude l2) {
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
        auto best = std::lower_bound(lats.begin(), lats.end(), s, [](Latitude l1, Latitude l2) { return l1 > l2; });
        s         = *best;
    }

    ASSERT(s <= n);
}


std::vector<double> Gaussian::calculateUnrotatedGridBoxLatitudeEdges() const {

    // grid-box edge latitudes are the accumulated Gaussian quadrature weights
    size_t Nj = N_ * 2;
    ASSERT(Nj > 1);

    auto& w = weights();
    ASSERT(w.size() == Nj);

    std::vector<double> edges(Nj + 1);
    auto f = edges.begin();
    auto b = edges.rbegin();

    *(f++) = Latitude::NORTH_POLE.value();
    *(b++) = Latitude::SOUTH_POLE.value();

    double wacc = -1.;
    for (size_t j = 0; j < N_; ++j, ++b, ++f) {
        wacc += 2. * w[j];
        double deg = util::radian_to_degree(std::asin(wacc));
        ASSERT(Latitude::SOUTH_POLE.value() <= deg && deg <= Latitude::NORTH_POLE.value());

        *b = deg;
        *f = -(*b);
    }

    return edges;
}


void Gaussian::fill(util::MeshGeneratorParameters& params) const {
    if (params.meshGenerator_.empty()) {
        params.meshGenerator_ = "structured";
    }

    const Latitude& s = bbox_.south();
    if (s <= latitudes().back() || s > Latitude::EQUATOR) {
        params.set("force_include_south_pole", true);
    }

    const Latitude& n = bbox_.north();
    if (n >= latitudes().front() || n < Latitude::EQUATOR) {
        params.set("force_include_north_pole", true);
    }
}


const std::vector<double>& Gaussian::latitudes(size_t N) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    ASSERT(N);
    auto latitudesIt = ml->find(N);
    if (latitudesIt == ml->end()) {
        eckit::Timer timer("Gaussian latitudes " + std::to_string(N), eckit::Log::debug<LibMir>());

        // calculate latitudes and insert in known-N-latitudes map
        std::vector<double> latitudes(N * 2);
        atlas::util::gaussian_latitudes_npole_spole(N, latitudes.data());

        ml->operator[](N) = latitudes;
        latitudesIt       = ml->find(N);
    }
    ASSERT(latitudesIt != ml->end());


    // these are the assumptions we expect from the Gaussian latitudes values
    auto& lats = latitudesIt->second;
    ASSERT(2 * N == lats.size());
    ASSERT(std::is_sorted(lats.begin(), lats.end(), [](double a, double b) { return a > b; }));

    return lats;
}


const std::vector<double>& Gaussian::weights(size_t N) {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);

    ASSERT(N);
    auto weightsIt = mw->find(N);
    if (weightsIt == mw->end()) {
        eckit::Timer timer("Gaussian quadrature weights " + std::to_string(N), eckit::Log::debug<LibMir>());

        // calculate quadrature weights and insert in known-N-weights map
        // FIXME: innefficient interface, latitudes are discarded
        std::vector<double> latitudes(N * 2);
        std::vector<double>& weights = (*mw)[N];
        weights.resize(N * 2);

        atlas::util::gaussian_quadrature_npole_spole(N, latitudes.data(), weights.data());

        weightsIt = mw->find(N);
    }
    ASSERT(weightsIt != mw->end());
    ASSERT(weightsIt->second.size() == 2 * N);

    return (*weightsIt).second;
}


const std::vector<double>& Gaussian::latitudes() const {
    return latitudes(N_);
}


const std::vector<double>& Gaussian::weights() const {
    return weights(N_);
}


}  // namespace repres
}  // namespace mir
