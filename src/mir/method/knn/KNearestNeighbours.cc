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
#include "eckit/log/BigNum.h"
#include "eckit/log/ProgressTimer.h"
#include "eckit/log/TraceTimer.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/utils/MD5.h"
#include "mir/config/LibMir.h"
#include "mir/method/knn/distance/DistanceWeighting.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Domain.h"


namespace mir {
namespace method {
namespace knn {


KNearestNeighbours::KNearestNeighbours(const param::MIRParametrisation& param) : MethodWeighted(param) {
    nClosest_ = 4;
    parametrisation_.get("nclosest", nClosest_);
    ASSERT(nClosest_);
}


KNearestNeighbours::~KNearestNeighbours() {
}


bool KNearestNeighbours::sameAs(const Method& other) const {
    const KNearestNeighbours* o = dynamic_cast<const KNearestNeighbours*>(&other);
    return o
            && (nClosest_ == o->nClosest_)
            && distanceWeighting().sameAs(o->distanceWeighting());
}


void KNearestNeighbours::hash(eckit::MD5& md5) const {
    md5 << nClosest_ << distanceWeighting();
}


void KNearestNeighbours::assemble(
        util::MIRStatistics& stats,
        WeightMatrix& W,
        const repres::Representation& in,
        const repres::Representation& out) const {

    // assemble with specific distance weighting method
    assemble(stats, W, in, out, distanceWeighting());
}


void KNearestNeighbours::assemble(
        util::MIRStatistics&,
        WeightMatrix& W,
        const repres::Representation& in,
        const repres::Representation& out,
        const distance::DistanceWeighting& distanceWeighting ) const {

    eckit::Log::debug<LibMir>() << *this << "::assemble (input: " << in << ", output: " << out << ")" << std::endl;
    eckit::TraceTimer<LibMir> timer("KNearestNeighbours::assemble");

    const size_t nbOutputPoints = out.numberOfPoints();

    const util::PointSearch sptree(parametrisation_, in);
    const util::Domain& inDomain = in.domain();


    double nearest = 0;
    double push_back = 0;

    // init structure used to fill in sparse matrix
    std::vector<WeightMatrix::Triplet> weights_triplets;
    weights_triplets.reserve(nbOutputPoints * nClosest_);

    std::vector<util::PointSearch::PointValueType> closest;
    std::vector<WeightMatrix::Triplet> triplets;

    {
        eckit::ProgressTimer progress("Locating", nbOutputPoints, "point", double(5), eckit::Log::debug<LibMir>());

        const eckit::ScopedPtr<repres::Iterator> it(out.iterator());
        size_t ip = 0;
        while (it->next()) {
            ASSERT(ip < nbOutputPoints);
            if (++progress) {
                eckit::Log::debug<LibMir>() << "KNearestNeighbours: k-d tree closest_n_points: " << nearest << "s, W push back:" << push_back << "s" << std::endl;
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
                    sptree.closestNPoints(p, nClosest_, closest);
                    nearest += timer.elapsed() - t;
                    ASSERT(closest.size() == nClosest_);
                }

                // calculate weights from distance
                distanceWeighting(ip, p, closest, triplets);
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

    eckit::Log::debug<LibMir>() << "Located " << eckit::BigNum(nbOutputPoints) << std::endl;

    // fill-in sparse matrix
    W.setFromTriplets(weights_triplets);
}


void KNearestNeighbours::print(std::ostream& out) const {
    out << "KNearestNeighbours[";
    MethodWeighted::print(out);
    out << ",nClosest=" << nClosest_
        << ",distanceWeighting=" << distanceWeighting()
        << "]";
}


}  // namespace knn
}  // namespace method
}  // namespace mir

