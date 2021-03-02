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


#include "mir/key/packing/SecondOrder.h"

#include <ostream>

#include "mir/util/Grib.h"


namespace mir {
namespace key {
namespace packing {


static PackingBuilder<SecondOrder> __packing("second-order", "so", false, true);


#if 0
bool SecondOrder::check(const repres::Representation& repres) const {
    auto n = repres.numberOfPoints();
    if (n < 4) {
        // NOTE: There is a bug in ecCodes if the user asks 1 value and select second-order
        // Once this fixed, remove this code
        Log::warning() << "Field has " << Log::Pretty(n, {"value"}) << " < 4, ignoring packer " << *this << std::endl;
        return false;
    }
    return true;
}
#endif


void SecondOrder::print(std::ostream& out) const {
    out << "SecondOrder[]";
}


void SecondOrder::fill(grib_info& info) const {
    savePacking(info, CODES_UTIL_PACKING_TYPE_GRID_SECOND_ORDER);
    saveAccuracy(info);
    saveEdition(info);
}


void SecondOrder::set(grib_handle* handle) const {
    setPacking(handle, "grid_second_order");
    setAccuracy(handle);
    setEdition(handle);
}


}  // namespace packing
}  // namespace key
}  // namespace mir
