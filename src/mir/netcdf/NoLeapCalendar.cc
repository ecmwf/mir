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
