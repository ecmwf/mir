// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/netcdf/Calendar.h"


namespace mir::netcdf {


class ProlepticCalendar : public Calendar {
public:
    ProlepticCalendar(const Variable&);
    ~ProlepticCalendar() override;

private:
    void print(std::ostream&) const override;
};


}  // namespace mir::netcdf
