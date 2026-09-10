// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/AllLeapCalendar.h"

#include <ostream>


namespace mir::netcdf {


AllLeapCalendar::AllLeapCalendar(const Variable& /*variable*/) {}


AllLeapCalendar::~AllLeapCalendar() = default;


void AllLeapCalendar::print(std::ostream& out) const {
    out << "AllLeapCalendar[]";
}


static const CodecBuilder<AllLeapCalendar> builder1("all_leap");
static const CodecBuilder<AllLeapCalendar> builder3("366_day");


}  // namespace mir::netcdf
