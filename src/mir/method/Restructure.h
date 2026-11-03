// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/method/MethodWeighted.h"
#include "mir/method/knn/pick/NClosestOrNearest.h"


namespace mir::method {


/**
 * @brief Restructure input values onto output grid, assuming input points are a (small) subset of the output points.
 *
 * Builds a point search on the output (target) grid and, for every input point, picks the nearest output point (the
 * first of the picked points, if multiple) receiving that input value with unit weight; all other output points are
 * set to missing value. At the poles, all (coincident) output points receive the value of the first input point picking
 * that pole.
 */
class Restructure final : public MethodWeighted {
public:
    explicit Restructure(const param::MIRParametrisation& param) : MethodWeighted(param), pick_(param) {}

private:
    void hash(eckit::MD5&) const override;
    void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in,
                  const repres::Representation& out) const override;
    bool sameAs(const Method&) const override;
    void json(eckit::JSON&) const override;
    void print(std::ostream&) const override;
    const char* type() const override;

    knn::pick::NClosestOrNearest pick_;
};


}  // namespace mir::method
