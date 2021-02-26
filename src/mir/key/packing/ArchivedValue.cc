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


#include "mir/key/packing/ArchivedValue.h"

#include <iostream>


namespace mir {
namespace key {
namespace packing {


static PackingBuilder<ArchivedValue> __packer1("archived-value");
static PackingBuilder<ArchivedValue> __packer2("av");  // For the lazy


ArchivedValue::~ArchivedValue() = default;


void ArchivedValue::print(std::ostream& out) const {
    out << "ArchivedValue[]";
}


void ArchivedValue::fill(grib_info&, const repres::Representation&) const {
    // Nothing to do
}


std::string ArchivedValue::type(const repres::Representation*) const {
    return "";
}


}  // namespace packing
}  // namespace key
}  // namespace mir
