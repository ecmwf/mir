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


#include "mir/compat/ProdgenArea.h"
#include "mir/util/Grib.h"


namespace mir {
namespace compat {

ProdgenArea::ProdgenArea(const std::string& name):
    GribCompatibility(name) {

    }

void ProdgenArea::execute(const param::MIRParametrisation&, grib_info& info) const {
    info.packing.deleteLocalDefinition = 1;
	NOTIMP;
}

void ProdgenArea::print(std::ostream& out) const {
    out << "prodgen-areas";
}

static ProdgenArea instance("prodgen-areas");

}  // namespace method
}  // namespace mir

