// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/netcdf/Dimension.h"


namespace mir::netcdf {


class VirtualInputDimension : public Dimension {
public:
    VirtualInputDimension(Dataset& owner, const std::string& name);
    ~VirtualInputDimension() override;

private:
    // Members

    // -- Methods

    // From Dimension
    void print(std::ostream&) const override;
    void clone(Dataset& owner) const override;
    int id() const override;
    void realDimensions(std::vector<size_t>& dims) const override;
};


}  // namespace mir::netcdf
