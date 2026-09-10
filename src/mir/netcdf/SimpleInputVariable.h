// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/netcdf/InputVariable.h"


namespace mir::netcdf {


class SimpleInputVariable : public InputVariable {
public:
    SimpleInputVariable(Dataset& owner, const std::string& name, int id, const std::vector<Dimension*>& dimensions);

    ~SimpleInputVariable() override;

private:
    std::vector<std::string> coordinates() const override;

    Variable* makeOutputVariable(Dataset& owner, const std::string& name,
                                 const std::vector<Dimension*>& dimensions) const override;

    Variable* makeDataVariable() override;
    Variable* makeCoordinateVariable() override;
    Variable* makeCellMethodVariable() override;

    void print(std::ostream&) const override;
    void validate() const override;
    bool identified() const override;

    Variable* addMissingCoordinates() override;
};


}  // namespace mir::netcdf
