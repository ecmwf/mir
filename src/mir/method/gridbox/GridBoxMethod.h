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

#include <vector>

#include "mir/method/MethodWeighted.h"
#include "mir/util/GridBox.h"


namespace mir::method::gridbox {


class GridBoxMethod : public MethodWeighted {
public:
    using MethodWeighted::MethodWeighted;

    struct GridBoxes : std::vector<util::GridBox> {
        explicit GridBoxes(const repres::Representation&, bool dual = false);
        double getLongestGridBoxDiagonal() const;
    };

private:
    void hash(eckit::MD5&) const override;
    bool sameAs(const Method&) const override;
    void json(eckit::JSON&) const override;
    void print(std::ostream&) const override;
    WeightMatrix::Check validateMatrixWeights() const override;
};


}  // namespace mir::method::gridbox
