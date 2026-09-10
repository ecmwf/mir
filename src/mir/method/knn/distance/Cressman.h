// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/method/knn/distance/DistanceWeighting.h"


namespace mir::method::knn::distance {


/**
 * Cressman, George P., An operational objective analysis system. Mon. Wea. Rev., 87, 367-374 (01 Oct 1959),
 * @ref http://dx.doi.org/10.1175/1520-0493(1959)087<0367:AOOAS>2.0.CO;2
 */
struct Cressman final : DistanceWeighting {
    explicit Cressman(const param::MIRParametrisation&);
    void operator()(size_t ip, const Point3& point, const std::vector<search::PointSearch::PointValueType>& neighbours,
                    std::vector<WeightMatrix::Triplet>& triplets) const override;

private:
    bool sameAs(const DistanceWeighting&) const override;
    void json(eckit::JSON&) const override;
    void print(std::ostream&) const override;
    void hash(eckit::MD5&) const override;

    const double r_;
    const double r2_;
    double power_;
};


}  // namespace mir::method::knn::distance
