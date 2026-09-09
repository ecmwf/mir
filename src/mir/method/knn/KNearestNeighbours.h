// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <memory>

#include "mir/method/MethodWeighted.h"


namespace mir::method::knn {
namespace distance {
class DistanceWeighting;
}
namespace pick {
class Pick;
}
}  // namespace mir::method::knn


namespace mir::method::knn {


class KNearestNeighbours : public MethodWeighted {
public:
    explicit KNearestNeighbours(const param::MIRParametrisation&);

    ~KNearestNeighbours() override;

    void hash(eckit::MD5&) const override;

protected:
    void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in,
                  const repres::Representation& out, const pick::Pick& pick, const distance::DistanceWeighting&) const;

    void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in,
                  const repres::Representation& out) const override;

    bool sameAs(const Method&) const override = 0;

private:
    void json(eckit::JSON&) const override;
    void print(std::ostream&) const override;

    const char* type() const override = 0;
    int version() const override;

    virtual const pick::Pick& pick() const                               = 0;
    virtual const distance::DistanceWeighting& distanceWeighting() const = 0;

    WeightMatrix::Check validateMatrixWeights() const override;
};


}  // namespace mir::method::knn
