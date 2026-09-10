// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/NoLeapCalendar.h"

#include <ostream>


namespace mir::netcdf {


NoLeapCalendar::NoLeapCalendar(const Variable& /*variable*/) {}


NoLeapCalendar::~NoLeapCalendar() = default;


void NoLeapCalendar::print(std::ostream& out) const {
    out << "NoLeapCalendar[]";
}


static const CodecBuilder<NoLeapCalendar> builder1("noleap");
static const CodecBuilder<NoLeapCalendar> builder2("365_day");


}  // namespace mir::netcdf
