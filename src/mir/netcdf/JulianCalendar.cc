// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/JulianCalendar.h"

#include <ostream>


namespace mir::netcdf {


JulianCalendar::JulianCalendar(const Variable& /*variable*/) {}


JulianCalendar::~JulianCalendar() = default;


void JulianCalendar::print(std::ostream& out) const {
    out << "JulianCalendar[]";
}


static const CodecBuilder<JulianCalendar> builder("julian");


}  // namespace mir::netcdf
