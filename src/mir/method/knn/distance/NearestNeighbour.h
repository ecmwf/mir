// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/method/knn/distance/DistanceWeighting.h"


namespace mir::method::knn::distance {


struct NearestNeighbour : DistanceWeighting {
    explicit NearestNeighbour(const param::MIRParametrisation&);

    void operator()(size_t ip, const Point3& point, const std::vector<search::PointSearch::PointValueType>& neighbours,
                    std::vector<WeightMatrix::Triplet>& triplets) const override;

private:
    bool sameAs(const DistanceWeighting&) const override;
    void json(eckit::JSON&) const override;
    void print(std::ostream&) const override;
    void hash(eckit::MD5&) const override;
};


}  // namespace mir::method::knn::distance
