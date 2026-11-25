// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/compat/SpectralComplexPacking.h"

#include <ostream>

#include "mir/util/Grib.h"


namespace mir::compat {


static const std::string NAME{"spectral-complex-packing"};
static const SpectralComplexPacking INSTANCE(NAME);


void SpectralComplexPacking::execute(const output::MIROutput&, const param::MIRParametrisation&, grib_handle*,
                                     grib_info& info) const {
    // packing as provided (by the input): spectral simple becomes spectral complex
    if (info.packing.packing == CODES_UTIL_PACKING_USE_PROVIDED &&
        info.packing.packing_type == CODES_UTIL_PACKING_TYPE_SPECTRAL_SIMPLE) {
        info.packing.packing_type = CODES_UTIL_PACKING_TYPE_SPECTRAL_COMPLEX;
    }
}


void SpectralComplexPacking::printParametrisation(std::ostream&, const param::MIRParametrisation&) const {}


bool SpectralComplexPacking::sameParametrisation(const param::MIRParametrisation&,
                                                 const param::MIRParametrisation&) const {
    return true;
}


void SpectralComplexPacking::initialise(const metkit::mars::MarsRequest&, std::map<std::string, std::string>&) const {}


void SpectralComplexPacking::print(std::ostream& out) const {
    out << NAME;
}


}  // namespace mir::compat
