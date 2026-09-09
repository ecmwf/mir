// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/netcdf/OutputVariable.h"


namespace mir::netcdf {


class CellMethodOutputVariable : public OutputVariable {
public:
    CellMethodOutputVariable(Dataset& owner, const std::string& name, const std::vector<Dimension*>& dimensions);
    ~CellMethodOutputVariable() override;

private:
    void merge(const Variable&, MergePlan&) override;
    void print(std::ostream&) const override;
};


}  // namespace mir::netcdf
