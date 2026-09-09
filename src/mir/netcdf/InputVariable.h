// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/netcdf/Variable.h"


namespace mir::netcdf {


class InputVariable : public Variable {
public:
    InputVariable(Dataset& owner, const std::string& name, int id, const std::vector<Dimension*>& dimensions);
    ~InputVariable() override;

protected:
    // Members
    int id_;

    // -- Methods

    virtual Variable* makeOutputVariable(Dataset& owner, const std::string& name,
                                         const std::vector<Dimension*>& dimensions) const = 0;

    // From variable
    void print(std::ostream&) const override;
    Variable* clone(Dataset& owner) const override;

    // From Endowed

    int varid() const override;
};


}  // namespace mir::netcdf
