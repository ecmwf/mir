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


#include "mir/method/knn/pick/NearestNeighbourWithLowestIndex.h"

#include <ostream>
#include <sstream>

#include "eckit/log/JSON.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::method::knn::pick {


NearestNeighbourWithLowestIndex::NearestNeighbourWithLowestIndex(const param::MIRParametrisation& param) {
    param.get("nclosest", nClosest_ = 4);
    ASSERT(nClosest_ > 0);
}


void NearestNeighbourWithLowestIndex::pick(const search::PointSearch& tree, const PointXYZ& point,
                                           neighbours_t& closest) const {
    if (nClosest_ == 1) {
        closest = {tree.closestPoint(point)};
        return;
    }


    // search for neighbour points
    neighbours_t neighbours;
    tree.closestNPoints(point, nClosest_, neighbours);
    ASSERT(neighbours.size() == nClosest_);


    // choose closest neighbour point with the lowest index (payload)
    size_t c        = 0;
    const double d2 = PointXYZ::distance2(point, neighbours.front().point());

    for (size_t j = 1; j < neighbours.size(); ++j) {
        if (eckit::types::is_strictly_greater(PointXYZ::distance2(point, neighbours[j].point()), d2)) {
            break;
        }
        if (neighbours[c].payload() > neighbours[j].payload()) {
            c = j;
        }
    }

    closest = {neighbours[c]};
}


size_t NearestNeighbourWithLowestIndex::n() const {
    return 1;
}


bool NearestNeighbourWithLowestIndex::sameAs(const Pick& other) const {
    const auto* o = dynamic_cast<const NearestNeighbourWithLowestIndex*>(&other);
    return (o != nullptr);
}


void NearestNeighbourWithLowestIndex::json(eckit::JSON& j) const {
    j.startObject();
    j << "type"
      << "nearest-neighbour-with-lowest-index";
    j << "nclosest" << nClosest_;
    j.endObject();
}


void NearestNeighbourWithLowestIndex::print(std::ostream& out) const {
    out << "NearestNeighbourWithLowestIndex[nclosest=" << nClosest_ << "]";
}


void NearestNeighbourWithLowestIndex::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


static const PickBuilder<NearestNeighbourWithLowestIndex> __pick("nearest-neighbour-with-lowest-index");


}  // namespace mir::method::knn::pick
