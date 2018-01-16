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
/// @date Apr 2015


#include "mir/output/MIROutput.h"

#include "mir/output/EmptyOutput.h"
#include "mir/output/GeoPointsFileOutput.h"
#include "mir/output/GribFileOutput.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace output {


MIROutput::MIROutput() {
}


MIROutput::~MIROutput() {
}


MIROutputFactory::MIROutputFactory() {
}


MIROutputFactory::~MIROutputFactory() {
}


MIROutput* MIROutputFactory::build(const std::string& name, const param::MIRParametrisation& parametrisation) {

    bool dryrun = false;
    if (parametrisation.get("dryrun", dryrun) && dryrun) {
        return new EmptyOutput();
    }

    if (parametrisation.has("griddef")) {
        return new GeoPointsFileOutput(name);
    }

    return new GribFileOutput(name);
}


}  // namespace output
}  // namespace mir

