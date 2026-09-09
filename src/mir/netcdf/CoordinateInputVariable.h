// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/netcdf/InputVariable.h"


namespace mir::netcdf {


class CoordinateInputVariable : public InputVariable {
public:
    CoordinateInputVariable(Dataset& owner, const std::string& name, int id, const std::vector<Dimension*>& dimensions);
    ~CoordinateInputVariable() override;

private:
    Variable* makeOutputVariable(Dataset& owner, const std::string& name,
                                 const std::vector<Dimension*>& dimensions) const override;

    Variable* makeCoordinateVariable() override;
    Variable* makeScalarCoordinateVariable() override;
    const char* kind() const override;

    void print(std::ostream&) const override;
};


}  // namespace mir::netcdf
