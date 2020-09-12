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


#include "mir/packing/ArchivedValue.h"

#include <iostream>


namespace mir {
namespace packing {


static ArchivedValue __packer1("archived-value");
static ArchivedValue __packer2("av");  // For the lazy


ArchivedValue::ArchivedValue(const std::string& name) : Packer(name) {}


ArchivedValue::~ArchivedValue() = default;


void ArchivedValue::print(std::ostream& out) const {
    out << "ArchivedValue[]";
}


void ArchivedValue::fill(grib_info&, const repres::Representation&, const param::MIRParametrisation&,
                         const param::MIRParametrisation&) const {
    // Nothing to do
}


}  // namespace packing
}  // namespace mir
