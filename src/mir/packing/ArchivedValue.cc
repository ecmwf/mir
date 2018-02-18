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



#include "mir/packing/ArchivedValue.h"
#include "mir/util/Grib.h"

#include <iostream>

namespace mir {
namespace packing {


ArchivedValue::ArchivedValue(const std::string &name):
    Packer(name) {
}


ArchivedValue::~ArchivedValue() {
}

void ArchivedValue::print(std::ostream& out) const {
    out << "ArchivedValue[]";
}

void ArchivedValue::fill(grib_info& info, const repres::Representation& ) const {
    // Nothing to do
}


namespace {
static ArchivedValue packing1("archived-value");
static ArchivedValue packing2("av"); // For the lazy

}


}  // namespace packing
}  // namespace mir

