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

    double r_;
    double r2_;
    double power_;
};


}  // namespace mir::method::knn::distance
