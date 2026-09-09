// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/netcdf/OutputVariable.h"


namespace mir::netcdf {


class SimpleOutputVariable : public OutputVariable {
public:
    SimpleOutputVariable(Dataset& owner, const std::string& name, const std::vector<Dimension*>& dimensions);
    ~SimpleOutputVariable() override;

private:
    void print(std::ostream&) const override;
};


}  // namespace mir::netcdf
