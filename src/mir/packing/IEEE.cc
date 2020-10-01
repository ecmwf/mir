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


#include "mir/packing/IEEE.h"

#include <iostream>
#include <sstream>

#include "eckit/exception/Exceptions.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Gridded.h"
#include "mir/util/Grib.h"


namespace mir {
namespace packing {


static IEEE __packer("ieee");


IEEE::IEEE(const std::string& name) : Packer(name) {}


IEEE::~IEEE() = default;


void IEEE::print(std::ostream& out) const {
    out << "IEEE[]";
}


void IEEE::fill(grib_info& info, const repres::Representation&, const param::MIRParametrisation& user,
                const param::MIRParametrisation& field) const {
    long bits = -1;
    if ((user.get("accuracy", bits) || field.get("accuracy", bits)) && (bits != 32 && bits != 64)) {
        std::ostringstream msg;
        msg << *this << ": only supports bitsPerValue 32 and 64 (from input, or as set by 'accuracy')";
        eckit::Log::error() << msg.str() << std::endl;
        throw eckit::UserError(msg.str());
    }

    info.packing.packing      = CODES_UTIL_PACKING_USE_PROVIDED;
    info.packing.packing_type = CODES_UTIL_PACKING_TYPE_IEEE;
}


std::string IEEE::packingType(const repres::Representation* repres) const {
    return dynamic_cast<const repres::Gridded*>(repres) != nullptr ? "grid_ieee" : "spectral_ieee";
}


}  // namespace packing
}  // namespace mir
