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


#include "mir/method/knn/pick/Distance.h"

#include "eckit/log/JSON.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::method::knn::pick {


Distance::Distance(const param::MIRParametrisation& param) {
    distance_ = 1.;
    param.get("distance", distance_);
    ASSERT(distance_ > 0.);
}


void Distance::pick(const search::PointSearch& tree, const Point3& p, Pick::neighbours_t& closest) const {
    tree.closestWithinRadius(p, distance_, closest);
}


size_t Distance::n() const {
    // NOTE: cannot be estimated
    return 4;
}


bool Distance::sameAs(const Pick& other) const {
    const auto* o = dynamic_cast<const Distance*>(&other);
    return (o != nullptr) && eckit::types::is_approximately_equal(distance_, o->distance_);
}


void Distance::json(eckit::JSON& j) const {
    j.startObject();
    j << "type"
      << "distance";
    j << "distance" << distance_;
    j.endObject();
}


void Distance::print(std::ostream& out) const {
    out << "Distance[" << distance_ << "]";
}


void Distance::hash(eckit::MD5& h) const {
    h.add(distance_);
}


static const PickBuilder<Distance> __pick("distance");


}  // namespace mir::method::knn::pick
