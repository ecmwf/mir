// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/netcdf/Attribute.h"


namespace mir::netcdf {


class InputAttribute : public Attribute {
public:
    InputAttribute(Endowed& owner, const std::string& name, Value* value);
    ~InputAttribute() override;

private:
    // From Atttribute
    void print(std::ostream&) const override;
    void clone(Endowed& owner) const override;
};


}  // namespace mir::netcdf
