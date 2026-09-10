// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/ProlepticCalendar.h"

#include <ostream>


namespace mir::netcdf {


ProlepticCalendar::ProlepticCalendar(const Variable& /*variable*/) {}


ProlepticCalendar::~ProlepticCalendar() = default;


void ProlepticCalendar::print(std::ostream& out) const {
    out << "ProlepticCalendar[]";
}


static const CodecBuilder<ProlepticCalendar> builder("proleptic_gregorian");


}  // namespace mir::netcdf
