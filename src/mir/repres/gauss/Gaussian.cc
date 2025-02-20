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
#include "mir/util/Trace.h"
#include "mir/util/Types.h"


namespace mir::repres {


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
    // grid-box edge latitudes are the mid-points of the Gaussian latitudes
    auto Nj = N_ * 2;
    ASSERT(Nj > 1);

    const auto& lats = latitudes();
    ASSERT(lats.size() == Nj);

    std::vector<double> edges(Nj + 1);
    auto f = edges.begin();
    auto b = edges.rbegin();
    (*f++) = Latitude::NORTH_POLE.value();
    (*b++) = Latitude::SOUTH_POLE.value();

    for (size_t j = 1; j < N_; ++j) {
        auto mid = (lats[j] + lats[j - 1]) / 2.;
        (*f++)   = mid;
        (*b++)   = -mid;
    }

    edges[N_] = 0.;  // exact value
    return edges;
}


void Gaussian::fillMeshGen(util::MeshGeneratorParameters& params) const {
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


void Gaussian::fillJob(api::MIRJob& job) const {
    auto d = domain();
    if (!d.isGlobal() || d.west() != Longitude::GREENWICH) {
        bbox_.fillJob(job);
    }
}


const std::vector<double>& Gaussian::latitudes(size_t N) {
    return eckit::geo::util::gaussian_latitudes(N, false);
}


const std::vector<double>& Gaussian::latitudes() const {
    return latitudes(N_);
}


void Gaussian::json(eckit::JSON& s) const {
    s << "N" << N_;
    s << "bbox" << bbox_;
    s << "angular_precision" << angularPrecision_;
}


}  // namespace mir::repres
