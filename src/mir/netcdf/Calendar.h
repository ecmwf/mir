// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/netcdf/Codec.h"


namespace mir::netcdf {
class Variable;
}  // namespace mir::netcdf


namespace mir::netcdf {


class Calendar : public Codec {
public:
    Calendar() = default;

private:
    bool timeAxis() const override;
};


}  // namespace mir::netcdf
