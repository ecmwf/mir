/*
 * (C) Copyright 1996-2015 ECMWF.
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


#include "mir/method/KNearestNeighbours.h"

#include <algorithm>
#include <limits>
#include "eckit/log/BigNum.h"
#include "eckit/log/ProgressTimer.h"
#include "eckit/log/TraceTimer.h"
#include "eckit/memory/ScopedPtr.h"
#include "mir/config/LibMir.h"
#include "mir/method/distance/DistanceWeighting.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Domain.h"


namespace mir {
namespace method {


KNearestNeighbours::KNearestNeighbours(const param::MIRParametrisation& param) :
    MethodWeighted(param) {
}


KNearestNeighbours::~KNearestNeighbours() {
}


void KNearestNeighbours::assemble(
        util::MIRStatistics& stats,
        WeightMatrix& W,
        const repres::Representation& in,
        const repres::Representation& out ) const {
    using namespace distance;

    // get distance weighting method
    eckit::ScopedPtr<const DistanceWeighting> calculateWeights(DistanceWeightingFactory::build(distanceWeighting(), parametrisation_));

    // assemble with specific distance weighting method
    assemble(stats, W, in, out, *calculateWeights);
}


void KNearestNeighbours::assemble(
        util::MIRStatistics&,
        WeightMatrix& W,
        const repres::Representation& in,
        const repres::Representation& out,
        const distance::DistanceWeighting& calculateWeights ) const {

    eckit::Log::debug<LibMir>() << "Nearest::assemble (input: " << in << ", output: " << out << ")" << std::endl;
    eckit::TraceTimer<LibMir> timer("Nearest::assemble");

    using eckit::geometry::LON;
    using eckit::geometry::LAT;

    const size_t nclosest = this->nclosest();
    const size_t out_npts = out.numberOfPoints();

    const util::PointSearch sptree(parametrisation_, in);

    const util::Domain& inDomain = in.domain();


    double nearest = 0;
    double push_back = 0;

    // init structure used to fill in sparse matrix
    std::vector<WeightMatrix::Triplet> weights_triplets;
    weights_triplets.reserve(out_npts * nclosest);
    eckit::Log::debug<LibMir>() << "Reserve " << eckit::BigNum(out_npts * nclosest) << std::endl;

    std::vector<util::PointSearch::PointValueType> closest;
    std::vector<WeightMatrix::Triplet> triplets;

    {
        eckit::ProgressTimer progress("Locating", out_npts, "point", double(5), eckit::Log::debug<LibMir>());

        const eckit::ScopedPtr<repres::Iterator> it(out.iterator());
        size_t ip = 0;
        while (it->next()) {
            ASSERT(ip < out_npts);
            ++progress;

            if (ip && (ip % 50000 == 0)) {
                eckit::Log::debug<LibMir>() << "Nearest: " << nearest << ", Push back:" << push_back << std::endl;
                sptree.statsPrint(eckit::Log::debug<LibMir>(), false);
                eckit::Log::debug<LibMir>() << std::endl;
                sptree.statsReset();
                nearest = push_back = 0;
            }

            if (inDomain.contains(it->pointUnrotated())) {

                // get the reference output point
                eckit::geometry::Point3 p(it->point3D());

                // 3D point to lookup
                {
                    double t = timer.elapsed();
                    sptree.closestNPoints(p, nclosest, closest);
                    nearest += timer.elapsed() - t;
                    ASSERT(closest.size() == nclosest);
                }

                // calculate weights from distance
                calculateWeights(ip, p, closest, triplets);
                ASSERT(!triplets.empty());

                // insert the interpolant weights into the global (sparse) interpolant matrix
                {
                    double t = timer.elapsed();
                    std::copy(triplets.begin(), triplets.end(), std::back_inserter(weights_triplets));
                    push_back += timer.elapsed() - t;
                }

            }

            ++ip;
        }
    }

    // fill-in sparse matrix
    W.setFromTriplets(weights_triplets);
}

}  // namespace method
}  // namespace mir

