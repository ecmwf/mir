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


namespace mir::method::gridbox {


class GridBoxMethod : public MethodWeighted {
public:
    using MethodWeighted::MethodWeighted;

private:
    void hash(eckit::MD5&) const override;
    bool sameAs(const Method&) const override;
    void json(eckit::JSON&) const override;
    void print(std::ostream&) const override;
    WeightMatrix::Check validateMatrixWeights() const override;
};


}  // namespace mir::method::gridbox
