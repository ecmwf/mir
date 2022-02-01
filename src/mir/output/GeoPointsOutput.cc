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


#include "mir/output/GeoPointsOutput.h"

#include <sstream>

#include "mir/util/Exceptions.h"


namespace mir {
namespace output {

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


}  // namespace output
}  // namespace mir
