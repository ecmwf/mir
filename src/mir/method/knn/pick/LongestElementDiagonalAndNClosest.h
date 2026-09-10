// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/method/knn/pick/Pick.h"


namespace mir::method::knn::pick {


struct LongestElementDiagonalAndNClosest : Pick {
    explicit LongestElementDiagonalAndNClosest(const param::MIRParametrisation&);

    void pick(const search::PointSearch&, const Point3&, neighbours_t&) const override;
    size_t n() const override;
    bool sameAs(const Pick&) const override;

private:
    void distance(const repres::Representation&) const override;
    void json(eckit::JSON&) const override;
    void print(std::ostream&) const override;
    void hash(eckit::MD5&) const override;

    const size_t nClosest_;
    mutable double distance_;
    mutable double distance2_;
    mutable bool nClosestFirst_;
};


}  // namespace mir::method::knn::pick
