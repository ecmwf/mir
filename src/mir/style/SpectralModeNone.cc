/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @date September 2017


#include "mir/style/SpectralModeNone.h"


namespace mir {
namespace style {


namespace {
static SpectralModeBuilder<SpectralModeNone> __spectral_mode("none");
}


SpectralModeNone::SpectralModeNone(const param::MIRParametrisation& parametrisation) : SpectralMode(parametrisation) {
}


bool SpectralModeNone::getGridname(std::string&) const {
    return false;
}

long SpectralModeNone::getTruncation() const {

    // TODO: this is temporary, no support yet for unstuctured grids
    if (parametrisation_.has("griddef")) {
        return 63L;
    }

    // Set truncation if manually specified
    long T = 0;
    parametrisation_.get("user.truncation", T);

    return T;
}


}  // namespace style
}  // namespace mir

