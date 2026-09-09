// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>
#include <vector>

#include "eckit/linalg/SparseMatrix.h"


namespace mir::method {


class WeightMatrix final : public eckit::linalg::SparseMatrix {
public:
    using Triplet = eckit::linalg::Triplet;
    using Scalar  = eckit::linalg::Scalar;
    using Size    = eckit::linalg::Size;

    struct Check {
        bool duplicates = true;
        bool bounds     = true;
        bool sum        = true;
    };

public:
    explicit WeightMatrix(SparseMatrix::Allocator* = nullptr);

    explicit WeightMatrix(const eckit::PathName&);

    WeightMatrix(Size rows, Size cols);

    void setFromTriplets(const std::vector<WeightMatrix::Triplet>&);

    void cleanup(const double& pruneEpsilon = 0);

    // Validate interpolation weights (default check matrix structure only)
    void validate(const char* when, Check = {true, false, false}) const;

private:
    void print(std::ostream&) const;

    friend std::ostream& operator<<(std::ostream& out, const WeightMatrix& m) {
        m.print(out);
        return out;
    }
};


}  // namespace mir::method
