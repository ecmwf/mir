// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/NoneCalendar.h"

#include <ostream>


namespace mir::netcdf {


NoneCalendar::NoneCalendar(const Variable& /*variable*/) {}


NoneCalendar::~NoneCalendar() = default;


void NoneCalendar::print(std::ostream& out) const {
    out << "NoneCalendar[]";
}


static const CodecBuilder<NoneCalendar> builder("none");


}  // namespace mir::netcdf
