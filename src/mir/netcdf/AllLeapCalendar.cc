/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// Baudouin Raoult - ECMWF Jan 2015

#include "mir/netcdf/AllLeapCalendar.h"

#include "mir/netcdf/Exceptions.h"
#include "mir/netcdf/OutputAttribute.h"
#include "mir/netcdf/Value.h"
#include "mir/netcdf/Variable.h"

#include <ostream>
#include <sstream>
#include <algorithm>

#include <netcdf.h>

namespace mir {
namespace netcdf {

AllLeapCalendar::AllLeapCalendar(const Variable& variable)
{
}

AllLeapCalendar::~AllLeapCalendar() {

}

void AllLeapCalendar::print(std::ostream &out) const {
    out << "AllLeapCalendar[]";
}

static CodecBuilder<AllLeapCalendar> builder1("all_leap");
static CodecBuilder<AllLeapCalendar> builder3("366_day");

}
}
