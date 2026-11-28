// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/compat/ProduceLargeConstantFields.h"

#include <ostream>

#include "mir/util/Grib.h"


namespace mir::compat {


static const std::string NAME{"produce-large-constant-fields"};
static const ProduceLargeConstantFields INSTANCE(NAME);


void ProduceLargeConstantFields::execute(const output::MIROutput&, const param::MIRParametrisation&, grib_handle*,
                                         grib_info& info) const {
    info.extra_set("produceLargeConstantFields", 1L);
}


void ProduceLargeConstantFields::printParametrisation(std::ostream&, const param::MIRParametrisation&) const {}


bool ProduceLargeConstantFields::sameParametrisation(const param::MIRParametrisation&,
                                                     const param::MIRParametrisation&) const {
    return true;
}


void ProduceLargeConstantFields::initialise(const metkit::mars::MarsRequest&,
                                            std::map<std::string, std::string>&) const {}


void ProduceLargeConstantFields::print(std::ostream& out) const {
    out << NAME;
}


}  // namespace mir::compat
