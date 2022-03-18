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


#include "mir/method/knn/KNearestNeighbours.h"

#include <algorithm>
#include <memory>

#include "eckit/utils/MD5.h"

#include "mir/method/knn/distance/DistanceWeighting.h"
#include "mir/method/knn/pick/Pick.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Trace.h"
#include "mir/util/Types.h"


namespace mir {
namespace method {
namespace knn {


KNearestNeighbours::KNearestNeighbours(const param::MIRParametrisation& param) : MethodWeighted(param) {}


KNearestNeighbours::~KNearestNeighbours() = default;


bool KNearestNeighbours::sameAs(const Method& other) const {
    const auto* o = dynamic_cast<const KNearestNeighbours*>(&other);
    return (o != nullptr) && pick().sameAs(o->pick()) && distanceWeighting().sameAs(o->distanceWeighting());
}


void KNearestNeighbours::hash(eckit::MD5& md5) const {
    md5 << pick();
    md5 << distanceWeighting();
}


void KNearestNeighbours::assemble(util::MIRStatistics& stats, WeightMatrix& W, const repres::Representation& in,
                                  const repres::Representation& out) const {
    // assemble with specific distance weighting method
    assemble(stats, W, in, out, pick(), distanceWeighting());
}


void KNearestNeighbours::assemble(util::MIRStatistics& /*unused*/, WeightMatrix& W, const repres::Representation& in,
                                  const repres::Representation& out, const pick::Pick& pick,
                                  const distance::DistanceWeighting& distanceWeighting) const {
    auto& log = Log::debug();

    log << *this << "::assemble (input: " << in << ", output: " << out << ")" << std::endl;
    trace::Timer timer("KNearestNeighbours::assemble");

    const size_t nbInputPoints  = in.numberOfPoints();
    const size_t nbOutputPoints = out.numberOfPoints();

    const search::PointSearch sptree(parametrisation_, in);
    const auto& inDomain = in.domain();
    pick.distance(in);


    // init structure used to fill in sparse matrix
    std::vector<WeightMatrix::Triplet> weights_triplets;
    weights_triplets.reserve(nbOutputPoints * pick.n());

    std::vector<search::PointSearch::PointValueType> closest;
    std::vector<WeightMatrix::Triplet> triplets;

    {
        trace::ProgressTimer progress("Locating", nbOutputPoints, {"point"});
        double search = 0;
        double insert = 0;

        for (const std::unique_ptr<repres::Iterator> it(out.iterator()); it->next();) {
            if (++progress) {
                log << "KNearestNeighbours: k-d tree"
                       "\n"
                       "search: "
                    << search
                    << "s"
                       "\n"
                       "insert: "
                    << insert
                    << "s"
                       "\n"
                    << sptree << std::endl;
                search = insert = 0;
            }

            if (inDomain.contains(it->pointRotated())) {

                // 3D point to lookup
                Point3 p(it->point3D());

                // search
                {
                    double t = timer.elapsed();
                    pick.pick(sptree, p, closest);
                    search += timer.elapsed(t);
                    if (closest.empty()) {
                        continue;
                    }
                }

                // calculate weights
                auto ip = it->index();
                ASSERT(ip < nbOutputPoints);

                distanceWeighting(ip, p, closest, triplets);
                ASSERT(!triplets.empty());

                // insert weights into the global (sparse) interpolant matrix
                {
                    double t = timer.elapsed();
                    std::copy(triplets.begin(), triplets.end(), std::back_inserter(weights_triplets));
                    insert += timer.elapsed(t);
                }
            }
        }
    }

    if (weights_triplets.empty()) {
        throw exception::SeriousBug("KNearestNeighbours: failed to interpolate");
    }

    // fill-in sparse matrix
    ASSERT_NONEMPTY_INTERPOLATION("KNearestNeighbours", !weights_triplets.empty());

    WeightMatrix M(nbOutputPoints, nbInputPoints, weights_triplets);
    W.swap(M);
}


void KNearestNeighbours::print(std::ostream& out) const {
    out << "KNearestNeighbours[";
    MethodWeighted::print(out);
    out << ",nearestMethod=" << pick() << ",distanceWeighting=" << distanceWeighting() << "]";
}


}  // namespace knn
}  // namespace method
}  // namespace mir
