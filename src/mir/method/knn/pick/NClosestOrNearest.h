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


#ifndef mir_method_knn_pick_NClosestOrNearest_h
#define mir_method_knn_pick_NClosestOrNearest_h

#include "mir/method/knn/pick/Pick.h"


namespace mir {
namespace method {
namespace knn {
namespace pick {


struct NClosestOrNearest : Pick {
    NClosestOrNearest(size_t nClosest, double distanceTolerance = 1.);
    NClosestOrNearest(const param::MIRParametrisation&);
    void pick(const search::PointSearch&, const Point3&, neighbours_t&) const override;
    size_t n() const override;
    bool sameAs(const Pick&) const override;
    void hash(eckit::MD5&) const override;

private:
    void print(std::ostream&) const override;
    size_t nClosest_;
    double distanceTolerance_;
    double distanceTolerance2_;
};


}  // namespace pick
}  // namespace knn
}  // namespace method
}  // namespace mir


#endif
