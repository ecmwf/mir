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

#include "mir/util/Grib.h"


namespace mir {
namespace packing {


static PackerBuilder<SecondOrder> __packer1("second-order");
static PackerBuilder<SecondOrder> __packer2("so");  // For the lazy


SecondOrder::SecondOrder(const param::MIRParametrisation& param) : Packer(param) {}


SecondOrder::~SecondOrder() = default;


void SecondOrder::print(std::ostream& out) const {
    out << "SecondOrder[]";
}


void SecondOrder::fill(grib_info& info, const repres::Representation&) const {
    info.packing.packing      = CODES_UTIL_PACKING_USE_PROVIDED;
    info.packing.packing_type = CODES_UTIL_PACKING_TYPE_GRID_SECOND_ORDER;
}


}  // namespace packing
}  // namespace mir
