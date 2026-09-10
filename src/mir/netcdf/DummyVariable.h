// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/netcdf/Variable.h"


namespace mir::netcdf {


class DummyVariable : public Variable {
public:
    DummyVariable(const Variable& parent);
    ~DummyVariable() override;

protected:
    const Variable& parent_;

    // -- Methods

    virtual Variable* makeOutputVariable(Dataset& owner, const std::string& name,
                                         const std::vector<Dimension*>& dimensions) const;
    virtual bool dummy() const;
    virtual bool sameAsDummy(const Variable&) const;

    // From variable

    void print(std::ostream&) const override;

    // From Endowed

    virtual int varid() const;
};


}  // namespace mir::netcdf
