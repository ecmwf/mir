// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/netcdf/Matrix.h"


namespace mir::netcdf {


class MergeCoordinateMatrix : public Matrix {
public:
    MergeCoordinateMatrix(Matrix* out, Matrix* in, size_t size);
    ~MergeCoordinateMatrix() override;

private:
    Matrix* out_;
    Matrix* in_;

    void print(std::ostream&) const override;

    void dumpTree(std::ostream&, size_t) const override;
};


}  // namespace mir::netcdf
