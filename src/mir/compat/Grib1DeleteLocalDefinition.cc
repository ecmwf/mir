// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/compat/Grib1DeleteLocalDefinition.h"

#include <ostream>

#include "mir/param/MIRParametrisation.h"
#include "mir/util/Grib.h"


namespace mir::compat {


static const std::string NAME{"grib1-delete-local-definition"};
static const Grib1DeleteLocalDefinition INSTANCE(NAME);


void Grib1DeleteLocalDefinition::execute(const output::MIROutput&, const param::MIRParametrisation& param, grib_handle*,
                                         grib_info& info) const {
    // the edition as encoded, otherwise as requested
    long edition = info.packing.editionNumber;
    if (edition == 0) {
        param.get("edition", edition);
    }

    if (edition == 1) {
        info.packing.deleteLocalDefinition = 1;
    }
}


void Grib1DeleteLocalDefinition::printParametrisation(std::ostream&, const param::MIRParametrisation&) const {}


bool Grib1DeleteLocalDefinition::sameParametrisation(const param::MIRParametrisation&,
                                                     const param::MIRParametrisation&) const {
    return true;
}


void Grib1DeleteLocalDefinition::initialise(const metkit::mars::MarsRequest&,
                                            std::map<std::string, std::string>&) const {}


void Grib1DeleteLocalDefinition::print(std::ostream& out) const {
    out << NAME;
}


}  // namespace mir::compat
