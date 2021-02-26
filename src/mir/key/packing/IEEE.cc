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


#include "mir/key/packing/IEEE.h"

#include <iostream>
#include <sstream>

#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Gridded.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Grib.h"
#include "mir/util/Log.h"


namespace mir {
namespace key {
namespace packing {


static PackingBuilder<IEEE> __packer("ieee");


IEEE::IEEE(const param::MIRParametrisation& user, const param::MIRParametrisation& field) : Packing(user, field) {
    long bits = -1;
    if ((user.get("accuracy", bits) || field.get("accuracy", bits)) && (bits != 32 && bits != 64)) {
        std::ostringstream msg;
        msg << *this << ": only supports bitsPerValue 32 and 64 (from input, or as set by 'accuracy')";
        Log::error() << msg.str() << std::endl;
        throw exception::UserError(msg.str());
    }
}


IEEE::~IEEE() = default;


void IEEE::print(std::ostream& out) const {
    out << "IEEE[]";
}


void IEEE::fill(grib_info& info, const repres::Representation&) const {
    info.packing.packing      = CODES_UTIL_PACKING_USE_PROVIDED;
    info.packing.packing_type = CODES_UTIL_PACKING_TYPE_IEEE;
}


std::string IEEE::type(const repres::Representation* repres) const {
    return dynamic_cast<const repres::Gridded*>(repres) != nullptr ? "grid_ieee" : "spectral_ieee";
}


}  // namespace packing
}  // namespace key
}  // namespace mir
