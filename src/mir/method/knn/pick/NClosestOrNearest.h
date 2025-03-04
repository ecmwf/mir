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


#pragma once

#include "mir/method/knn/pick/Pick.h"


namespace mir::method::knn::pick {


struct NClosestOrNearest : Pick {
    explicit NClosestOrNearest(size_t nClosest, double distanceTolerance = 1.);
    explicit NClosestOrNearest(const param::MIRParametrisation&);

    void pick(const search::PointSearch&, const Point3&, neighbours_t&) const override;
    size_t n() const override;
    bool sameAs(const Pick&) const override;
    void hash(eckit::MD5&) const override;

private:
    void json(eckit::JSON&) const override;
    void print(std::ostream&) const override;

    size_t nClosest_;
    double distanceTolerance_;
    double distanceTolerance2_;
};


}  // namespace mir::method::knn::pick
