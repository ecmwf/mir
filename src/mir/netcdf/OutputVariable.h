// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/netcdf/Variable.h"


namespace mir::netcdf {


class OutputVariable : public Variable {
public:
    OutputVariable(Dataset& owner, const std::string& name, const std::vector<Dimension*>& dimensions);
    ~OutputVariable() override;

private:
    // -- Members

    mutable int id_;
    mutable bool created_;

    // From variable

    void print(std::ostream&) const override;
    void create(int nc) const override;
    void save(int nc) const override;

    // From Endowed

    int varid() const override;
};


}  // namespace mir::netcdf
