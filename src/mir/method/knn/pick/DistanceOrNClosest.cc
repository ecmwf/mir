/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/knn/pick/DistanceOrNClosest.h"

#include "eckit/exception/Exceptions.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace method {
namespace knn {
namespace pick {


DistanceOrNClosest::DistanceOrNClosest(const param::MIRParametrisation& param) :
    nclosest_(param) {
    distance_ = 1.;
    param.get("distance", distance_);
    ASSERT(distance_ > 0.);
}


void DistanceOrNClosest::pick(const search::PointSearch& tree,
                              const eckit::geometry::Point3& p,
                              Pick::neighbours_t& closest) const {
    tree.closestWithinRadius(p, distance_, closest);
    if (closest.size() < nclosest_.n()) {
        nclosest_.pick(tree, p, closest);
    }
}


size_t DistanceOrNClosest::n() const {
    return nclosest_.n();
}


bool DistanceOrNClosest::sameAs(const Pick& other) const {
    auto o = dynamic_cast<const DistanceOrNClosest*>(&other);
    return o
            && eckit::types::is_approximately_equal(distance_, o->distance_)
            && nclosest_.sameAs(o->nclosest_);
}


void DistanceOrNClosest::print(std::ostream& out) const {
    out << "DistanceOrNClosest["
            "nclosest=" << nclosest_
        << ",distance=" << distance_
        << "]";
}


void DistanceOrNClosest::hash(eckit::MD5& h) const {
    std::ostringstream s;
    s << *this;
    h.add(s.str());
}


static PickBuilder<DistanceOrNClosest> __pick("distance-or-nclosest");


}  // namespace pick
}  // namespace knn
}  // namespace method
}  // namespace mir

