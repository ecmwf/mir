// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/knn/KNearestNeighbours.h"

#include <algorithm>
#include <memory>
#include <variant>

#include "eckit/geo/Grid.h"
#include "eckit/geo/Search.h"
#include "eckit/log/JSON.h"
#include "eckit/utils/MD5.h"

#include "mir/method/knn/distance/DistanceWeighting.h"
#include "mir/method/knn/pick/Pick.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/LonLatToXYZ.h"
#include "mir/util/Search.h"
#include "mir/util/Trace.h"
#include "mir/util/Types.h"


namespace mir::method::knn {


KNearestNeighbours::KNearestNeighbours(const param::MIRParametrisation& param) : MethodWeighted(param) {}


KNearestNeighbours::~KNearestNeighbours() = default;


bool KNearestNeighbours::sameAs(const Method& other) const {
    const auto* o = dynamic_cast<const KNearestNeighbours*>(&other);
    return (o != nullptr) && pick().sameAs(o->pick()) && distanceWeighting().sameAs(o->distanceWeighting()) &&
           MethodWeighted::sameAs(other);
}


void KNearestNeighbours::hash(eckit::MD5& md5) const {
    MethodWeighted::hash(md5);
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

    const size_t nbOutputPoints = out.numberOfPoints();

    const auto sptree    = util::make_search(parametrisation_, in);
    const auto& inDomain = in.geographicDomain();
    pick.distance(in);

    const util::LonLatToXYZ point3(in.canonicalGrid().figure(), poleDisplacement(in));

    // init structure used to fill in sparse matrix
    std::vector<WeightMatrix::Triplet> weights_triplets;
    weights_triplets.reserve(nbOutputPoints * pick.n());

    eckit::geo::Search::Neighbours closest;
    std::vector<WeightMatrix::Triplet> triplets;

    {
        trace::ProgressTimer progress("Locating", nbOutputPoints, {"point"});
        double search = 0;
        double insert = 0;

        const auto& points = out.canonicalGrid();
        for (auto it = points.cbegin(); it; ++it) {
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
                    << *sptree << std::endl;
                search = insert = 0;
            }

            if (const auto q = std::get<PointLonLat>(*it); inDomain.contains(q)) {

                // 3D point to lookup
                const auto p = point3(q);

                // search
                {
                    double t = timer.elapsed();
                    pick.pick(*sptree, p, closest);
                    search += timer.elapsed(t);
                    if (closest.empty()) {
                        continue;
                    }
                }

                // calculate weights
                auto ip = it.index();
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
    W.setFromTriplets(weights_triplets);
}


void KNearestNeighbours::json(eckit::JSON& j) const {
    MethodWeighted::json(j);

    pick().json(j);
    distanceWeighting().json(j);
}


void KNearestNeighbours::print(std::ostream& out) const {
    out << "KNearestNeighbours[";
    MethodWeighted::print(out);
    out << ",nearestMethod=" << pick() << ",distanceWeighting=" << distanceWeighting() << "]";
}


int KNearestNeighbours::version() const {
    return 0;
}


WeightMatrix::Check KNearestNeighbours::validateMatrixWeights() const {
    return distanceWeighting().validateMatrixWeights();
}


}  // namespace mir::method::knn
