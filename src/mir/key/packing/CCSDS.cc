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


#include "mir/key/packing/CCSDS.h"

#include <iostream>

#include "mir/repres/Gridded.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"


namespace mir {
namespace key {
namespace packing {


static PackingBuilder<CCSDS> __packer("ccsds");


CCSDS::~CCSDS() = default;


void CCSDS::print(std::ostream& out) const {
    out << "CCSDS[]";
}


void CCSDS::fill(grib_info& info, const repres::Representation&) const {
    info.packing.packing      = CODES_UTIL_PACKING_USE_PROVIDED;
    info.packing.packing_type = CODES_UTIL_PACKING_TYPE_CCSDS;
}


std::string CCSDS::type(const repres::Representation* repres) const {
    if (dynamic_cast<const repres::Gridded*>(repres) != nullptr) {
        return "grid_ccsds";
    }
    NOTIMP;
}


}  // namespace packing
}  // namespace key
}  // namespace mir
