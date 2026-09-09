// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/ThreeSixtyCalendar.h"

#include <ostream>


namespace mir::netcdf {


ThreeSixtyCalendar::ThreeSixtyCalendar(const Variable& /*variable*/) {}


ThreeSixtyCalendar::~ThreeSixtyCalendar() = default;


void ThreeSixtyCalendar::print(std::ostream& out) const {
    out << "ThreeSixtyCalendar[]";
}


static const CodecBuilder<ThreeSixtyCalendar> builder("360_day");


}  // namespace mir::netcdf
