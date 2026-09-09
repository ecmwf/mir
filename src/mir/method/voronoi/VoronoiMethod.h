// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/method/MethodWeighted.h"
#include "mir/method/knn/pick/NClosestOrNearest.h"


namespace mir::method::voronoi {


class VoronoiMethod : public MethodWeighted {
public:
    explicit VoronoiMethod(const param::MIRParametrisation&);

private:
    void hash(eckit::MD5&) const override;
    void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in,
                  const repres::Representation& out) const override;
    bool sameAs(const Method&) const override;
    void print(std::ostream&) const override;
    WeightMatrix::Check validateMatrixWeights() const override;
    int version() const override;

    knn::pick::NClosestOrNearest pick_;
};


}  // namespace mir::method::voronoi
