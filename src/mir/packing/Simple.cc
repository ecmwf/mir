/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015



#include "mir/packing/Simple.h"
#include "mir/util/Grib.h"
#include "mir/repres/Representation.h"

#include <iostream>

namespace mir {
namespace packing {


Simple::Simple(const std::string &name):
    Packer(name) {
}


Simple::~Simple() = default;

void Simple::print(std::ostream& out) const {
    out << "Simple[]";
}

void Simple::fill(grib_info& info, const repres::Representation& repres) const {
    info.packing.packing = GRIB_UTIL_PACKING_USE_PROVIDED;
    repres.setSimplePacking(info);
}


namespace {
static Simple packing("simple");

}


}  // namespace packing
}  // namespace mir

