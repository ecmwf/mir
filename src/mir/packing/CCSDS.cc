/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/packing/CCSDS.h"

#include <iostream>

#include "mir/util/Grib.h"


namespace mir {
namespace packing {


namespace {
static CCSDS packing("ccsds");
}


CCSDS::CCSDS(const std::string& name) : Packer(name) {}


CCSDS::~CCSDS() = default;


void CCSDS::print(std::ostream& out) const {
    out << "CCSDS[]";
}


void CCSDS::fill(grib_info& info, const repres::Representation&) const {
    info.packing.packing      = GRIB_UTIL_PACKING_USE_PROVIDED;
    info.packing.packing_type = GRIB_UTIL_PACKING_TYPE_CCSDS;
}


}  // namespace packing
}  // namespace mir
