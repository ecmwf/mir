/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


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
