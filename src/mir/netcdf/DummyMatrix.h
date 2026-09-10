// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/netcdf/Matrix.h"


namespace mir::netcdf {
class Variable;
}  // namespace mir::netcdf


namespace mir::netcdf {


class DummyMatrix : public Matrix {
public:
    DummyMatrix(const Variable&);
    ~DummyMatrix() override;

private:
    Matrix* other_;

    // Methods

    void print(std::ostream&) const override;
};


}  // namespace mir::netcdf
