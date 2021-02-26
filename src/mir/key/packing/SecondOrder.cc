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

#include <iostream>

#include "mir/repres/Gridded.h"
#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"


namespace mir {
namespace key {
namespace packing {


static PackingBuilder<SecondOrder> __packer1("second-order");
static PackingBuilder<SecondOrder> __packer2("so");  // For the lazy


SecondOrder::~SecondOrder() = default;


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


void SecondOrder::print(std::ostream& out) const {
    out << "SecondOrder[]";
}


void SecondOrder::fill(grib_info& info, const repres::Representation& repres) const {
    if (check(repres)) {
        info.packing.packing      = CODES_UTIL_PACKING_USE_PROVIDED;
        info.packing.packing_type = CODES_UTIL_PACKING_TYPE_GRID_SECOND_ORDER;
    }
}


std::string SecondOrder::type(const repres::Representation* repres) const {
    if (dynamic_cast<const repres::Gridded*>(repres) != nullptr) {
        return check(*repres) ? "grid_second_order" : "";
    }
    NOTIMP;
}


}  // namespace packing
}  // namespace key
}  // namespace mir
