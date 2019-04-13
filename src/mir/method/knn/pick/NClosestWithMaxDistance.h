/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_method_knn_pick_NClosestWithMaxDistance_h
#define mir_method_knn_pick_NClosestWithMaxDistance_h

#include "mir/method/knn/pick/Pick.h"
#include "mir/method/knn/pick/Distance.h"


namespace mir {
namespace method {
namespace knn {
namespace pick {


struct NClosestWithMaxDistance : Pick {
    NClosestWithMaxDistance(const param::MIRParametrisation&);
    void pick(const search::PointSearch&, const Point3&, neighbours_t&) const;
    size_t n() const;
    bool sameAs(const Pick&) const;
private:
    void print(std::ostream&) const;
    void hash(eckit::MD5&) const;
    Distance distance_;
    size_t nclosest_;
};


}  // namespace pick
}  // namespace knn
}  // namespace method
}  // namespace mir


#endif

