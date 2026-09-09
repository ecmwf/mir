// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/netcdf/Attribute.h"


namespace mir::netcdf {


class OutputAttribute : public Attribute {
public:
    OutputAttribute(Endowed& owner, const std::string& name, Value* value);
    ~OutputAttribute() override;

private:
    // -- Members

    bool valid_;

    // From Atttribute
    void create(int nc) const override;
    void print(std::ostream&) const override;
    void clone(Endowed& owner) const override;
    void merge(const Attribute&) override;
    void invalidate() override;
};


}  // namespace mir::netcdf
