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


#include "mir/packing/IEEE.h"

#include <iostream>

#include "mir/util/Grib.h"


namespace mir {
namespace packing {


static IEEE packing("ieee");


IEEE::IEEE(const std::string& name) : Packer(name) {}


IEEE::~IEEE() = default;


void IEEE::print(std::ostream& out) const {
    out << "IEEE[]";
}


void IEEE::fill(grib_info& info, const repres::Representation&) const {
    info.packing.packing      = CODES_UTIL_PACKING_USE_PROVIDED;
    info.packing.packing_type = CODES_UTIL_PACKING_TYPE_IEEE;
}


}  // namespace packing
}  // namespace mir
