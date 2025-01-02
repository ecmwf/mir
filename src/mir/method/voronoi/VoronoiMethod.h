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
    void json(eckit::JSON&) const override;
    void print(std::ostream&) const override;
    WeightMatrix::Check validateMatrixWeights() const override;
    const char* name() const override;
    int version() const override;

    knn::pick::NClosestOrNearest pick_;
};


}  // namespace mir::method::voronoi
