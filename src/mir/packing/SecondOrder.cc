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


#include "mir/packing/SecondOrder.h"

#include <iostream>

#include "eckit/exception/Exceptions.h"

#include "mir/repres/Gridded.h"
#include "mir/repres/Representation.h"
#include "mir/util/Grib.h"
#include "mir/util/Pretty.h"


namespace mir {
namespace packing {


static SecondOrder __packer1("second-order");
static SecondOrder __packer2("so");  // For the lazy


SecondOrder::SecondOrder(const std::string& name) : Packer(name) {}


SecondOrder::~SecondOrder() = default;


void SecondOrder::print(std::ostream& out) const {
    out << "SecondOrder[]";
}


void SecondOrder::fill(grib_info& info, const repres::Representation& repres, const param::MIRParametrisation&,
                       const param::MIRParametrisation&) const {
    auto n = repres.numberOfPoints();
    if (n < 4) {
        // NOTE: There is a bug in ecCodes if the user asks 1 value and select second-order
        // Once this fixed, remove this code
        eckit::Log::warning() << "Field has " << Pretty(n, {"value"}) << ", ignoring packer " << *this << std::endl;
        return;
    }

    info.packing.packing      = CODES_UTIL_PACKING_USE_PROVIDED;
    info.packing.packing_type = CODES_UTIL_PACKING_TYPE_GRID_SECOND_ORDER;
}


std::string SecondOrder::packingType(const repres::Representation* repres) const {
    if (dynamic_cast<const repres::Gridded*>(repres) != nullptr) {
        return "grid_second_order";
    }
    NOTIMP;
}


}  // namespace packing
}  // namespace mir
