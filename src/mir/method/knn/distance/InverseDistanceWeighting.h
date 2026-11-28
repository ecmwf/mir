// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <string>

#include "mir/method/knn/distance/DistanceWeighting.h"


namespace mir::method::knn::distance {


struct InverseDistanceWeighting : DistanceWeighting {
protected:
    InverseDistanceWeighting(const param::MIRParametrisation&, double power, const std::string& name);
    void operator()(size_t ip, const Point3& point, const std::vector<search::PointSearch::PointValueType>& neighbours,
                    std::vector<WeightMatrix::Triplet>& triplets) const override;

    void json(eckit::JSON& j) const override;

    double power() const { return power_; }

private:
    bool sameAs(const DistanceWeighting&) const override;
    void print(std::ostream&) const override;
    void hash(eckit::MD5&) const override;

    double power_;
    double halfPower_;
    std::string name_;
};


}  // namespace mir::method::knn::distance
