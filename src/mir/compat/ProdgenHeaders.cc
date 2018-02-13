/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date May 2015


#include "mir/compat/ProdgenHeaders.h"
#include "mir/util/Grib.h"


namespace mir {
namespace compat {

ProdgenHeaders::ProdgenHeaders(const std::string& name):
    GribCompatibility(name) {

    }

void ProdgenHeaders::execute(const param::MIRParametrisation&, grib_handle*, grib_info& info) const {
    info.packing.deleteLocalDefinition = 1;
}

void ProdgenHeaders::print(std::ostream& out) const {
    out << "no-local-extension";
}

static ProdgenHeaders instance("no-local-extension");

}  // namespace method
}  // namespace mir

