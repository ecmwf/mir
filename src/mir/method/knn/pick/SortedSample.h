// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/method/knn/pick/Pick.h"
#include "mir/method/knn/pick/Sample.h"


namespace mir::method::knn::pick {


struct SortedSample : Pick {
    explicit SortedSample(const param::MIRParametrisation&);

    void pick(const search::PointSearch&, const Point3&, neighbours_t&) const override;
    size_t n() const override;
    bool sameAs(const Pick&) const override;
    void hash(eckit::MD5&) const override;

private:
    void json(eckit::JSON&) const override;
    void print(std::ostream&) const override;

    const Sample sample_;
};


}  // namespace mir::method::knn::pick
