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


#include "mir/key/packing/PNG.h"

#include <ostream>

#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"


namespace mir {
namespace key {
namespace packing {


static PackingBuilder<PNG> __packing("png", false, true);


void PNG::fill(grib_info& info) const {
    ASSERT(!userPacking());
    saveAccuracy(info);
    saveEdition(info);
}


void PNG::set(grib_handle* handle) const {
    setPacking(handle, "grid_png");
    setAccuracy(handle);
    setEdition(handle);
}


void PNG::print(std::ostream& out) const {
    out << "PNG[]";
}


}  // namespace packing
}  // namespace key
}  // namespace mir
