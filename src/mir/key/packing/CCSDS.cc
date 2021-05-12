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

#include "mir/util/Grib.h"


namespace mir {
namespace key {
namespace packing {


static PackingBuilder<CCSDS> __packing("ccsds", false, true);


void CCSDS::fill(const repres::Representation*, grib_info& info) const {
    Packing::fill(info, CODES_UTIL_PACKING_TYPE_CCSDS);
}


void CCSDS::set(const repres::Representation*, grib_handle* handle) const {
    Packing::set(handle, "grid_ccsds");
}


}  // namespace packing
}  // namespace key
}  // namespace mir
