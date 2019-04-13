/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/knn/pick/DistanceWithMinNClosest.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace method {
namespace knn {
namespace pick {


DistanceWithMinNClosest::DistanceWithMinNClosest(const param::MIRParametrisation& param) :
    nclosest_(param) {
    distance_ = 1.;
    param.get("distance", distance_);
    ASSERT(distance_ > 0.);
}


void DistanceWithMinNClosest::pick(const search::PointSearch& tree,
                    const eckit::geometry::Point3& p,
                    Pick::neighbours_t& closest) const {
    tree.closestWithinRadius(p, distance_, closest);
    if (closest.size() < nclosest_.n()) {
        nclosest_.pick(tree, p, closest);
    }
}


size_t DistanceWithMinNClosest::n() const {
    return nclosest_.n();
}


bool DistanceWithMinNClosest::sameAs(const Pick& other) const {
    auto o = dynamic_cast<const DistanceWithMinNClosest*>(&other);
    return o
            && eckit::types::is_approximately_equal(distance_, o->distance_)
            && nclosest_.sameAs(o->nclosest_);
}


void DistanceWithMinNClosest::print(std::ostream& out) const {
    out << "DistanceWithMinNClosest["
            "nclosest=" << nclosest_
        << ",distance=" << distance_
        << "]";
}


void DistanceWithMinNClosest::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


static PickBuilder<DistanceWithMinNClosest> __pick("distance-with-min-nclosest");


}  // namespace pick
}  // namespace knn
}  // namespace method
}  // namespace mir

