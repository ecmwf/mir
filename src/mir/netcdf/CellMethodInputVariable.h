// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/netcdf/InputVariable.h"


namespace mir::netcdf {


class CellMethodInputVariable : public InputVariable {
public:
    CellMethodInputVariable(Dataset& owner, const std::string& name, int id, const std::vector<Dimension*>& dimensions);
    ~CellMethodInputVariable() override;

private:
    // From InputVariable

    Variable* makeOutputVariable(Dataset& owner, const std::string& name,
                                 const std::vector<Dimension*>& dimensions) const override;

    // From variable

    void print(std::ostream&) const override;
    const char* kind() const override;
};


}  // namespace mir::netcdf
