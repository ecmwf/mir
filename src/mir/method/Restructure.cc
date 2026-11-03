// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/Restructure.h"

#include <algorithm>
#include <iterator>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <vector>

#include "eckit/geo/PointXYZ.h"
#include "eckit/log/JSON.h"
#include "eckit/utils/MD5.h"

#include "mir/method/knn/pick/NClosestOrNearest.h"
#include "mir/method/knn/pick/Pick.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/search/PointSearch.h"
#include "mir/util/Domain.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Point2ToPoint3.h"
#include "mir/util/Trace.h"
#include "mir/util/Types.h"


namespace mir::method {


static const std::string RESTRUCTURE{"restructure"};


void Restructure::hash(eckit::MD5& md5) const {
    MethodWeighted::hash(md5);
    md5 << pick_;
}


void Restructure::assemble(util::MIRStatistics& /*unused*/, WeightMatrix& W, const repres::Representation& in,
                           const repres::Representation& out) const {
    auto& log = Log::debug();

    log << *this << "::assemble (input: " << in << ", output: " << out << ")" << std::endl;
    trace::Timer timer("Restructure::assemble");

    const auto Nc = in.numberOfPoints();
    const auto Nr = out.numberOfPoints();

    // search the output (target) points, for every input point
    const util::Point2ToPoint3 point3(out, poleDisplacement());
    const search::PointSearch tree(parametrisation_, out);
    const auto& outDomain = out.geographicDomain();

    // pole points (first input point picking a pole sets all its coincident output points)
    struct PolePoints : std::set<size_t> {
        PolePoints(const search::PointSearch& kdtree, double lat, bool skip) : is_assigned(skip) {
            if (!skip) {
                std::vector<search::PointSearch::PointValueType> closest;
                kdtree.closestWithinRadius(repres::Iterator::point_3D({lat, 0.}), eckit::geo::PointXYZ::EPS, closest);
                for (const auto& c : closest) {
                    insert(c.payload());
                }
            }
        }
        bool is_assigned;
    } np(tree, Latitude::NORTH_POLE.value(), !out.includesNorthPole()),
        sp(tree, Latitude::SOUTH_POLE.value(), !out.includesSouthPole());

    // output point (row) to input point (column), unassigned rows are set to missing value
    std::vector<WeightMatrix::Triplet> triplets;
    triplets.reserve(Nc + np.size() + sp.size());  // for efficiency

    {
        trace::ProgressTimer progress("Locating", Nc, {"point"});

        std::vector<search::PointSearch::PointValueType> closest;
        for (const std::unique_ptr<repres::Iterator> it(in.iterator()); it->next();) {
            ++progress;

            if (!outDomain.contains(it->pointRotated())) {
                continue;
            }

            pick_.pick(tree, point3(*(*it)), closest);
            if (closest.empty()) {
                continue;
            }

            const auto c = it->index();
            ASSERT(c < Nc);

            const auto r = closest.front().payload();
            ASSERT(r < Nr);

            if (!np.is_assigned) {
                if (np.count(r) != 0) {
                    np.is_assigned = true;
                    for (const auto p : np) {
                        triplets.emplace_back(p, c, 1.);
                    }
                    continue;
                }
            }

            if (!sp.is_assigned) {
                if (sp.count(r) != 0) {
                    sp.is_assigned = true;
                    for (const auto p : sp) {
                        triplets.emplace_back(p, c, 1.);
                    }
                    continue;
                }
            }

            triplets.emplace_back(r, c, 1.);
        }
    }

    // fill-in sparse matrix (triplets ordered by row), first input point picking an output point wins
    std::sort(triplets.begin(), triplets.end());
    auto last =
        std::unique(triplets.begin(), triplets.end(), [](const auto& a, const auto& b) { return a.row() == b.row(); });

    if (auto collisions = static_cast<size_t>(std::distance(last, triplets.end())); collisions > 0) {
        Log::warning() << "Restructure: " << Log::Pretty(collisions, {"input point"})
                       << " picking an already picked output point, which is ignored" << std::endl;
    }

    triplets.erase(last, triplets.end());

    ASSERT_NONEMPTY_INTERPOLATION("Restructure", !triplets.empty());
    W.setFromTriplets(triplets);
}


bool Restructure::sameAs(const Method& other) const {
    const auto* o = dynamic_cast<const Restructure*>(&other);
    return (o != nullptr) && (pick_.sameAs(o->pick_)) && MethodWeighted::sameAs(other);
}


void Restructure::json(eckit::JSON& j) const {
    MethodWeighted::json(j);
    static_cast<const knn::pick::Pick&>(pick_).json(j);
}


void Restructure::print(std::ostream& out) const {
    out << "Restructure[";
    MethodWeighted::print(out);
    out << ",nearestMethod=" << pick_ << "]";
}


const char* Restructure::type() const {
    return RESTRUCTURE.c_str();
}


static const MethodBuilder<Restructure> __method(RESTRUCTURE);


}  // namespace mir::method
