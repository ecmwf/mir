// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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

    const size_t nClosest_;
    const double distanceTolerance_;
    const double distanceTolerance2_;
};


}  // namespace mir::method::knn::pick
