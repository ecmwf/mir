// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/netcdf/Variable.h"


namespace mir::netcdf {


class DummyInputVariable : public Variable {
public:
    DummyInputVariable(Dataset& owner, const Variable& parent);
    ~DummyInputVariable() override;

protected:
    const Variable& parent_;

    // -- Methods

    virtual Variable* makeOutputVariable(Dataset& owner, const std::string& name,
                                         const std::vector<Dimension*>& dimensions) const;
    bool dummy() const override;
    bool sameAsDummy(const Variable&) const override;
    const std::string& ncname() const override;

    // From variable

    void print(std::ostream&) const override;

    // From Endowed

    int varid() const override;
};


}  // namespace mir::netcdf
