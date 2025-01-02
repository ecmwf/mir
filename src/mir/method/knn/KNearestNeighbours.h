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

    const char* name() const override = 0;
    int version() const override;

    virtual const pick::Pick& pick() const                               = 0;
    virtual const distance::DistanceWeighting& distanceWeighting() const = 0;

    WeightMatrix::Check validateMatrixWeights() const override;
};


}  // namespace mir::method::knn
