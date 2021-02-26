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


#include "mir/key/packing/Simple.h"

#include <iostream>

#include "mir/repres/Gridded.h"
#include "mir/repres/Representation.h"
#include "mir/util/Grib.h"


namespace mir {
namespace key {
namespace packing {


static PackingBuilder<Simple> __packer("simple");


Simple::~Simple() = default;


void Simple::print(std::ostream& out) const {
    out << "Simple[]";
}


void Simple::fill(grib_info& info, const repres::Representation& repres) const {
    info.packing.packing = CODES_UTIL_PACKING_USE_PROVIDED;
    repres.setSimplePacking(info);
}


std::string Simple::type(const repres::Representation* repres) const {
    return dynamic_cast<const repres::Gridded*>(repres) != nullptr ? "grid_simple" : "spectral_simple";
}


}  // namespace packing
}  // namespace key
}  // namespace mir
