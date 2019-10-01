/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/knn/distance/NearestNeighbourWithLowestIndex.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace method {
namespace knn {
namespace distance {


NearestNeighbourWithLowestIndex::NearestNeighbourWithLowestIndex(const param::MIRParametrisation& parametrisation) {

    distanceTolerance_ = 1.;
    parametrisation.get("distance-tolerance", distanceTolerance_);
    ASSERT(distanceTolerance_ >= 0.);

    distanceTolerance2_ = distanceTolerance_ * distanceTolerance_;
}


void NearestNeighbourWithLowestIndex::operator()(
        size_t ip,
        const Point3& point,
        const std::vector<search::PointSearch::PointValueType>& neighbours,
        std::vector<WeightMatrix::Triplet>& triplets ) const {

    const size_t nbPoints = neighbours.size();
    ASSERT(nbPoints);

    // choose closest neighbour point with the lowest index (payload)
    size_t jp = neighbours.front().payload();
    const double d2 = Point3::distance2(point, neighbours.front().point());

    for (size_t j = 1; j < nbPoints; ++j) {
        if (eckit::types::is_strictly_greater(Point3::distance2(point, neighbours[j].point()), d2, distanceTolerance2_)) {
            break;
        }
        if (jp > neighbours[j].payload()) {
            jp = neighbours[j].payload();
        }
    }

    triplets.assign(1, WeightMatrix::Triplet(ip, jp, 1.));
}


bool NearestNeighbourWithLowestIndex::sameAs(const DistanceWeighting& other) const {
    auto o = dynamic_cast<const NearestNeighbourWithLowestIndex*>(&other);
    return o && eckit::types::is_approximately_equal(distanceTolerance_, o->distanceTolerance_);
}


void NearestNeighbourWithLowestIndex::print(std::ostream& out) const {
    out << "NearestNeighbourWithLowestIndex["
            "distanceTolerance=" << distanceTolerance_
        << "]";
}


void NearestNeighbourWithLowestIndex::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


static DistanceWeightingBuilder<NearestNeighbourWithLowestIndex> __distance("nearest-neighbour-with-lowest-index");


}  // namespace distance
}  // namespace knn
}  // namespace method
}  // namespace mir

