// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/output/GeoPointsOutput.h"

#include <sstream>

#include "mir/util/Exceptions.h"


namespace mir::output {

// See https://software.ecmwf.int/wiki/display/METV/Geopoints


GeoPointsOutput::GeoPointsOutput() : once_(true) {}


GeoPointsOutput::~GeoPointsOutput() = default;


bool GeoPointsOutput::once() {
    if (!once_) {
        std::ostringstream oss;
        oss << "GeoPointsOutput: attempt to write more than once to " << *this;
        throw exception::SeriousBug(oss.str());
    }

    once_ = false;
    return true;
}


bool GeoPointsOutput::sameParametrisation(const param::MIRParametrisation& /*unused*/,
                                          const param::MIRParametrisation& /*unused*/) const {
    return true;
}


bool GeoPointsOutput::printParametrisation(std::ostream& /*unused*/,
                                           const param::MIRParametrisation& /*unused*/) const {
    return false;
}


}  // namespace mir::output
