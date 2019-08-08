/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date May 2015


#include "mir/method/knn/KNearestNeighbours.h"

#include <algorithm>
#include <memory>

#include "eckit/log/Plural.h"
#include "eckit/log/ProgressTimer.h"
#include "eckit/log/Seconds.h"
#include "eckit/log/TraceTimer.h"
#include "eckit/utils/MD5.h"

#include "mir/config/LibMir.h"
#include "mir/method/knn/distance/DistanceWeighting.h"
#include "mir/method/knn/pick/Pick.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Domain.h"


namespace mir {
namespace method {
namespace knn {


KNearestNeighbours::KNearestNeighbours(const param::MIRParametrisation& param) : MethodWeighted(param) {

    std::string distanceWeighting;
    param.get("distance-weighting", distanceWeighting = "inverse-distance-weighting-squared");

    distanceWeighting_.reset(distance::DistanceWeightingFactory::build(distanceWeighting, param));
    ASSERT(distanceWeighting_);
}


KNearestNeighbours::~KNearestNeighbours() = default;


bool KNearestNeighbours::sameAs(const Method& other) const {
    auto o = dynamic_cast<const KNearestNeighbours*>(&other);
    return o && distanceWeighting().sameAs(o->distanceWeighting());
}


void KNearestNeighbours::hash(eckit::MD5& h) const {
    h << distanceWeighting();
}

void KNearestNeighbours::assembleCustomised(util::MIRStatistics& stats, WeightMatrix& W,
                                            const repres::Representation& in, const repres::Representation& out,
                                            const pick::Pick& pick,
                                            const distance::DistanceWeighting& distanceWeighting) const {
    eckit::Log::debug<LibMir>() << *this << "::assemble (input: " << in << ", output: " << out << ")" << std::endl;
    eckit::TraceTimer<LibMir> timer("KNearestNeighbours::assemble");

    const size_t nbOutputPoints = out.numberOfPoints();

    const search::PointSearch sptree(parametrisation_, in);
    const util::Domain& inDomain = in.domain();


    double nearest   = 0;
    double push_back = 0;
    distance::DistanceWeighting::neighbours_t closest;
    distance::DistanceWeighting::weights_t weights;

    // init structure used to fill in sparse matrix
    std::vector<WeightMatrix::Triplet> triplets;
    triplets.reserve(nbOutputPoints * pick.n());

    {
        eckit::ProgressTimer progress("Locating", nbOutputPoints, "point", double(5), eckit::Log::debug<LibMir>());

        const std::unique_ptr<repres::Iterator> it(out.iterator());
        size_t ip = 0;
        while (it->next()) {
            ASSERT(ip < nbOutputPoints);
            if (++progress) {
                sptree.statsPrint(eckit::Log::debug<LibMir>()
                                      << "KNearestNeighbours: k-d tree nearest: " << eckit::Seconds(nearest)
                                      << " push back: " << eckit::Seconds(push_back) << std::endl,
                                  false);
                eckit::Log::debug<LibMir>() << std::endl;
                sptree.statsReset();
                nearest = push_back = 0;
            }

            if (inDomain.contains(it->pointRotated())) {

                // get the reference output point
                Point3 p(it->point3D());

                // pick nearest points
                double t = timer.elapsed();
                pick.pick(sptree, ip, p, closest);
                nearest += timer.elapsed() - t;

                // calculate weights from distance
                distanceWeighting(p, closest, weights);
                ASSERT(!weights.empty() && weights.size() <= closest.size());

                // insert the interpolant weights into the global (sparse) interpolant matrix
                t = timer.elapsed();
                triplets.reserve(triplets.size() + weights.size());
                for (size_t w = 0; w < weights.size(); ++w) {
                    triplets.emplace_back(WeightMatrix::Triplet{ip, closest[w].payload(), weights[w]});
                }
                push_back += timer.elapsed() - t;
            }

            ++ip;
        }
    }

    if (triplets.empty()) {
        throw eckit::SeriousBug("KNearestNeighbours: failed to interpolate");
    }

    // fill-in sparse matrix
    W.setFromTriplets(triplets);
}


const distance::DistanceWeighting& KNearestNeighbours::distanceWeighting() const {
    ASSERT(distanceWeighting_);
    return *distanceWeighting_;
}


bool KNearestNeighbours::canIntroduceMissingValues() const {
    return true;
}


}  // namespace knn
}  // namespace method
}  // namespace mir
