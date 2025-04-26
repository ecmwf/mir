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


#include "mir/method/knn/pick/Sample.h"

#include <cstdlib>

#include "eckit/log/JSON.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::method::knn::pick {


Sample::Sample(const param::MIRParametrisation& param) {
    distance_ = 1.;
    param.get("distance", distance_);
    ASSERT(distance_ > 0.);

    nClosest_ = 4;
    param.get("nclosest", nClosest_);
    ASSERT(nClosest_ > 0);
}


void Sample::pick(const search::PointSearch& tree, const PointXYZ& p, Pick::neighbours_t& closest) const {
    tree.closestWithinRadius(p, distance_, closest);
    if (closest.size() <= nClosest_) {
        return;
    }

    // reservoir sampling in-place (output not ordered by distance)
    for (size_t n = nClosest_; n < closest.size(); ++n) {
        auto r = size_t(std::rand()) % n;
        if (r < nClosest_) {
            closest[r] = closest[n];
        }
    }

    // closest.resize(nClosest_);  // FIXME: better than below (but has black magic)
    closest.erase(closest.begin() + long(nClosest_), closest.end());
}


size_t Sample::n() const {
    return nClosest_;
}


bool Sample::sameAs(const Pick& other) const {
    const auto* o = dynamic_cast<const Sample*>(&other);
    return (o != nullptr) && eckit::types::is_approximately_equal(distance_, o->distance_);
}


void Sample::json(eckit::JSON& j) const {
    j.startObject();
    j << "type"
      << "sample";
    j << "nclosest" << nClosest_;
    j << "distance" << distance_;
    j.endObject();
}


void Sample::print(std::ostream& out) const {
    out << "Sample[nclosest=" << nClosest_ << ",distance=" << distance_ << "]";
}


void Sample::hash(eckit::MD5& h) const {
    h << "sample";
    h << nClosest_;
    h << distance_;
}


static const PickBuilder<Sample> __pick("sample");


}  // namespace mir::method::knn::pick
