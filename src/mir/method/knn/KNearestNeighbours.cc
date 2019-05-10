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

    std::string name = "nclosest-or-nearest";
    param.get("nearest-method", name);

    picker_.reset(pick::PickFactory::build(name, param));
    ASSERT(picker_);
}


KNearestNeighbours::~KNearestNeighbours() = default;


bool KNearestNeighbours::sameAs(const Method& other) const {
    auto o = dynamic_cast<const KNearestNeighbours*>(&other);
    return o
            && picker_->sameAs(*(o->picker_))
            && distanceWeighting().sameAs(o->distanceWeighting());
}


void KNearestNeighbours::hash(eckit::MD5& md5) const {
    md5 << *picker_;
    md5 << distanceWeighting();
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

    const search::PointSearch sptree(parametrisation_, in);
    const util::Domain& inDomain = in.domain();


    double nearest = 0;
    double push_back = 0;

    // init structure used to fill in sparse matrix
    std::vector<WeightMatrix::Triplet> weights_triplets;
    weights_triplets.reserve(nbOutputPoints * picker_->n());

    std::vector<search::PointSearch::PointValueType> closest;
    std::vector<WeightMatrix::Triplet> triplets;

    {
        eckit::ProgressTimer progress("Locating", nbOutputPoints, "point", double(5), eckit::Log::debug<LibMir>());

        const std::unique_ptr<repres::Iterator> it(out.iterator());
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

            if (inDomain.contains(it->pointRotated())) {

                // get the reference output point
                Point3 p(it->point3D());

                // 3D point to lookup
                {
                    double t = timer.elapsed();
                    picker_->pick(sptree, p, closest);
                    nearest += timer.elapsed() - t;
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

    if (weights_triplets.empty()) {
        throw eckit::SeriousBug("KNearestNeighbours: failed to interpolate");
    }

    // fill-in sparse matrix
    W.setFromTriplets(weights_triplets);
}


void KNearestNeighbours::print(std::ostream& out) const {
    out << "KNearestNeighbours[";
    MethodWeighted::print(out);
    out << ",nearestMethod=" << (*picker_)
        << ",distanceWeighting=" << distanceWeighting()
        << "]";
}


bool KNearestNeighbours::canIntroduceMissingValues() const {
    return true;
}


}  // namespace knn
}  // namespace method
}  // namespace mir

