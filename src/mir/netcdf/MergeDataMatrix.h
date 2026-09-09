// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/netcdf/Matrix.h"


namespace mir::netcdf {


class MergeDataMatrix : public Matrix {
public:
    MergeDataMatrix(Matrix* out, Matrix* in, size_t size);
    ~MergeDataMatrix() override;

private:
    Matrix* out_;
    Matrix* in_;

    void print(std::ostream&) const override;

    void dumpTree(std::ostream&, size_t) const override;
};


}  // namespace mir::netcdf
