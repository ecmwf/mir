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


#include "mir/netcdf/ProlepticCalendar.h"

#include "mir/netcdf/Exceptions.h"
#include "mir/netcdf/OutputAttribute.h"
#include "mir/netcdf/Value.h"
#include "mir/netcdf/Variable.h"

#include <algorithm>
#include <ostream>
#include <sstream>

#include <netcdf.h>

namespace mir {
namespace netcdf {

ProlepticCalendar::ProlepticCalendar(const Variable& variable) {}

ProlepticCalendar::~ProlepticCalendar() = default;

void ProlepticCalendar::print(std::ostream& out) const {
    out << "ProlepticCalendar[]";
}

static CodecBuilder<ProlepticCalendar> builder("proleptic_gregorian");

}  // namespace netcdf
}  // namespace mir
