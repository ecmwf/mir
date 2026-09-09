// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/netcdf/Dimension.h"


namespace mir::netcdf {


class OutputDimension : public Dimension {
public:
    OutputDimension(Dataset& owner, const std::string& name, size_t len);
    ~OutputDimension() override;

private:
    mutable int id_;
    mutable bool created_;
    bool grown_;

    // -- Methods

    // From Dimension
    void print(std::ostream&) const override;
    void create(int nc) const override;
    int id() const override;
    void grow(size_t) override;
};


}  // namespace mir::netcdf
